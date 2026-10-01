package com.ziv.snfiletools

import android.graphics.BitmapFactory
import android.util.Log
import java.io.ByteArrayOutputStream
import java.io.File
import org.json.JSONObject
import supernote.generated.annotations.SupernotePluginAsync
import supernote.generated.annotations.SupernotePluginExport

// Research helper for one X2 ink layer on a white background. This firmware
// produces opaque PNG even for type=0; flattening other layers or a template
// would change semantics. Reserved grayscale collisions are reported explicitly.
class ExportBitmap {
    @SupernotePluginExport
    @SupernotePluginAsync
    suspend fun encodeExportedBitmap(pngPath: String, rlePath: String, width: Int, height: Int): String {
        require(width > 0 && height > 0 && width.toLong() * height <= 20_000_000)
        require(pngPath.endsWith(".png") && rlePath.endsWith(".rle"))
        val bitmap = checkNotNull(BitmapFactory.decodeFile(pngPath)) { "Cannot decode exported PNG" }
        try {
            require(bitmap.width == width && bitmap.height == height) { "Export dimensions differ from page" }
            val pixels = IntArray(width * height)
            bitmap.getPixels(pixels, 0, width, 0, 0, width, height)
            val alphaCounts = IntArray(256)
            val grayCounts = IntArray(256)
            for (pixel in pixels) {
                alphaCounts[pixel ushr 24]++
                if (pixel ushr 24 != 0) grayCounts[(pixel ushr 16) and 255]++
            }
            Log.i("SNfiletoolsNative", "PNG pixel histogram: " + JSONObject()
                .put("transparent", alphaCounts[0]).put("opaque", alphaCounts[255])
                .put("grays", JSONObject().apply {
                    for (gray in 0..255) if (grayCounts[gray] > 0) put(gray.toString(), grayCounts[gray])
                }))
            val output = ByteArrayOutputStream()
            val reserved = setOf(0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x9d, 0x9e, 0xc9, 0xca)
            var previous = -1
            var length = 0
            var ink = 0
            var background = 0
            var nonWhite = 0
            var adjusted = 0
            var maxGrayAdjustment = 0
            var partialAlpha = 0
            fun emit() {
                var remaining = length
                while (remaining >= 16384) {
                    output.write(previous)
                    output.write(0xff)
                    remaining -= 16384
                }
                while (remaining > 0) {
                    val count = minOf(remaining, 128)
                    output.write(previous)
                    output.write(count - 1)
                    remaining -= count
                }
            }
            for (pixel in pixels) {
                val alpha = pixel ushr 24
                val red = (pixel ushr 16) and 255
                val green = (pixel ushr 8) and 255
                val blue = pixel and 255
                val code = if (alpha == 0) {
                    background++
                    0x62
                } else {
                    require(red == green && green == blue) { "Unsupported colored export pixel" }
                    // Note layer bitmaps cannot store partial alpha. This
                    // diagnostic's fixture has a white background; retain its
                    // visible gray by compositing antialiasing against white.
                    val gray = (red * alpha + 255 * (255 - alpha) + 127) / 255
                    if (alpha != 255) partialAlpha++
                    if (gray != 255) { ink++; nonWhite++ }
                    when (gray) {
                        0 -> 0x61
                        255 -> { background++; 0x62 }
                        else -> {
                            if (gray in reserved) {
                                var delta = 1
                                while (gray - delta in reserved && gray + delta in reserved) delta++
                                adjusted++
                                maxGrayAdjustment = maxOf(maxGrayAdjustment, delta)
                                if (gray - delta !in reserved) gray - delta else gray + delta
                            } else gray
                        }
                    }
                }
                if (code != previous) {
                    if (length > 0) emit()
                    previous = code
                    length = 0
                }
                length++
            }
            if (length > 0) emit()
            check(nonWhite > 0) { "Export contains no visible ink; bitmap will not be replaced" }
            check(background > pixels.size / 2) { "Export differs from the prepared white-background fixture" }
            File(rlePath).writeBytes(output.toByteArray())
            return JSONObject().put("width", width).put("height", height)
                .put("inkPixels", ink).put("nonWhitePixels", nonWhite).put("backgroundPixels", background)
                .put("adjustedGrayPixels", adjusted).put("maxGrayAdjustment", maxGrayAdjustment)
                .put("partialAlphaPixels", partialAlpha)
                .put("opaqueExport", alphaCounts[255] == pixels.size)
                .put("rleBytes", output.size()).toString()
        } finally {
            bitmap.recycle()
        }
    }
}
