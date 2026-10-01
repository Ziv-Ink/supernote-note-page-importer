package com.ziv.snfiletools

import java.io.File
import supernote.generated.annotations.SupernotePluginExport
import supernote.generated.annotations.SupernotePluginAsync

// The JS caller passes PluginManager.getPluginDirPath(). Store only diagnostic
// paths and phase, never note content. JVM exports run on the async worker.
@SupernotePluginExport
@SupernotePluginAsync
suspend fun getTotalPathTestState(pluginDirectory: String): String {
    val file = File(pluginDirectory, "snfiletools-totalpath-test.json")
    return if (file.exists()) file.readText() else ""
}

@SupernotePluginExport
@SupernotePluginAsync
suspend fun setTotalPathTestState(pluginDirectory: String, state: String) {
    File(pluginDirectory, "snfiletools-totalpath-test.json").writeText(state)
}
