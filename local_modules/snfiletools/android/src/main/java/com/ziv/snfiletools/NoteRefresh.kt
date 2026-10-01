package com.ziv.snfiletools

import android.content.Context
import android.content.Intent
import android.content.ComponentName
import android.content.ServiceConnection
import android.os.Binder
import android.os.IBinder
import android.os.Parcel
import android.os.Process
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withTimeout
import kotlinx.coroutines.delay
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import supernote.generated.annotations.SupernotePluginExport
import supernote.generated.annotations.SupernotePluginAsync

class NoteRefresh(private val context: Context) {
    // React Native timers pause when openFile brings Notes to the foreground.
    // A coroutine delay keeps this explicitly started operation progressing.
    @SupernotePluginExport
    @SupernotePluginAsync
    suspend fun waitForNoteRefresh(milliseconds: Int) {
        require(milliseconds in 1..10000)
        delay(milliseconds.toLong())
    }

    // Firmware-specific, narrowly scoped Binder probe. Parcel layout and codes
    // come from this device's IPluginClient/ClientRequest/PluginAPIResponse.
    @SupernotePluginExport
    @SupernotePluginAsync
    suspend fun clearNoteFileCache(path: String, page: Int): Boolean {
        require(path.endsWith(".note") && page >= 0)
        var bound = false
        var connection: ServiceConnection? = null
        try {
            return withTimeout(10000) {
                suspendCancellableCoroutine<Boolean> { continuation ->
                    val requestId = System.identityHashCode(continuation)
                    val callback = object : Binder() {
                        override fun onTransact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean {
                            if (code != 1) return super.onTransact(code, data, reply, flags)
                            // Exceptions must reach the suspended JS call rather
                            // than escape Android's callback thread.
                            try {
                                data.enforceInterface("com.ratta.supernote.plugincorelib.callback.IRequestClientCallback")
                                check(data.readInt() == 1) { "Missing cache-clear response" }
                                check(data.readInt() == requestId) { "Unexpected response ID" }
                                val success = data.readByte().toInt() != 0
                                val result = data.readValue(javaClass.classLoader)
                                val errorClass = data.readString()
                                check(errorClass == null) {
                                    "Notes error ${data.readInt()}: ${data.readString()}"
                                }
                                check(success && result is Boolean) { "Unexpected cache-clear response" }
                                if (continuation.isActive) continuation.resume(result)
                            } catch (error: Exception) {
                                if (continuation.isActive) continuation.resumeWithException(error)
                            }
                            return true
                        }
                    }
                    connection = object : ServiceConnection {
                        override fun onServiceConnected(name: ComponentName, service: IBinder) {
                            val data = Parcel.obtain()
                            val reply = Parcel.obtain()
                            try {
                                check(service.interfaceDescriptor == "com.ratta.supernote.plugincorelib.IPluginClient")
                                data.writeInterfaceToken("com.ratta.supernote.plugincorelib.IPluginClient")
                                data.writeInt(1)
                                data.writeInt(requestId)
                                data.writeString("clearFileCache")
                                data.writeList(listOf(path, page))
                                data.writeStrongBinder(callback)
                                check(service.transact(2, data, reply, 0)) { "Notes rejected Binder transaction" }
                                reply.readException()
                            } catch (error: Exception) {
                                if (continuation.isActive) continuation.resumeWithException(error)
                            } finally {
                                reply.recycle()
                                data.recycle()
                            }
                        }
                        override fun onServiceDisconnected(name: ComponentName) {
                            if (continuation.isActive) continuation.resumeWithException(IllegalStateException("Notes disconnected"))
                        }
                    }
                    bound = context.bindService(
                        Intent().setComponent(ComponentName("com.ratta.supernote.note", "com.ratta.supernote.pluginclient.PluginClientService")),
                        connection!!,
                        Context.BIND_AUTO_CREATE
                    )
                    check(bound) { "Could not bind Notes service" }
                }
            }
        } finally {
            if (bound) context.unbindService(connection!!)
        }
    }

    // Ordinary platform delivery, using only PluginHost's own permissions.
    // Delivery is asynchronous; bitmap/save checks must verify the effect.
    @SupernotePluginExport
    @SupernotePluginAsync
    suspend fun requestNoteRedraw(): String {
        context.sendBroadcast(
            Intent("com.ratta.supernote.launcher.flashscreen")
                .setPackage("com.ratta.supernote.note")
        )
        return "Sent Notes flashscreen broadcast from uid ${Process.myUid()}"
    }
}
