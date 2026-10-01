@file:Suppress("unused")
package supernote.generated.adapters

import com.facebook.react.bridge.ReactApplicationContext
import supernote.generated.runtime.SupernoteCoroutineBridge

object Identity_43b471d30e6b9bf9dbda {
  @JvmStatic
  fun identityHash(value: Any): Int = System.identityHashCode(value)
  @JvmStatic
  fun newList(): MutableList<Any?> = mutableListOf()
  @JvmStatic
  fun listAdd(list: MutableList<Any?>, value: Any?) { list.add(value) }
  @JvmStatic
  fun listSize(list: List<*>): Int = list.size
  @JvmStatic
  fun listGet(list: List<*>, index: Int): Any? = list[index]
  @JvmStatic fun boxBoolean(value: Boolean): Any = value
  @JvmStatic fun boxInt(value: Int): Any = value
  @JvmStatic fun boxLong(value: Long): Any = value
  @JvmStatic fun boxFloat(value: Float): Any = value
  @JvmStatic fun boxDouble(value: Double): Any = value
  @JvmStatic fun unboxBoolean(value: Any): Boolean = value as Boolean
  @JvmStatic fun unboxInt(value: Any): Int = value as Int
  @JvmStatic fun unboxLong(value: Any): Long = value as Long
  @JvmStatic fun unboxFloat(value: Any): Float = value as Float
  @JvmStatic fun unboxDouble(value: Any): Double = value as Double
}

object Adapter_72f3cb2919f47e9f0eb9 {
  @JvmStatic
  fun invoke(context: ReactApplicationContext): `com`.`ziv`.`snfiletools`.`ExportBitmap` =
      `com`.`ziv`.`snfiletools`.`ExportBitmap`()
}

object Adapter_c88de5ed49abe06d2574 {
  @JvmStatic
  fun invoke(owner: `com`.`ziv`.`snfiletools`.`ExportBitmap`, arg0: ByteArray, arg1: ByteArray, arg2: Int, arg3: Int, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    owner.`encodeExportedBitmap`(arg0.decodeToString(throwOnInvalidSequence = true), arg1.decodeToString(throwOnInvalidSequence = true), arg2, arg3).encodeToByteArray()
  }
}

object Adapter_4c155f9e040e4f0c13b0 {
  @JvmStatic
  fun invoke(arg0: ByteArray): ByteArray = com.ziv.snfiletools.`greetFromJvm`(arg0.decodeToString(throwOnInvalidSequence = true)).encodeToByteArray()
}

object Adapter_7ef685164ed9789997ba {
  @JvmStatic
  fun invoke(context: ReactApplicationContext): `com`.`ziv`.`snfiletools`.`NoteRefresh` =
      `com`.`ziv`.`snfiletools`.`NoteRefresh`(context)
}

object Adapter_52224597b558df0cc584 {
  @JvmStatic
  fun invoke(owner: `com`.`ziv`.`snfiletools`.`NoteRefresh`, arg0: Int, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    owner.`waitForNoteRefresh`(arg0); null
  }
}

object Adapter_d298b6e2e2d9e7cd7b1d {
  @JvmStatic
  fun invoke(owner: `com`.`ziv`.`snfiletools`.`NoteRefresh`, arg0: ByteArray, arg1: Int, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    owner.`clearNoteFileCache`(arg0.decodeToString(throwOnInvalidSequence = true), arg1)
  }
}

object Adapter_c7f222126e065a1f88be {
  @JvmStatic
  fun invoke(owner: `com`.`ziv`.`snfiletools`.`NoteRefresh`, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    owner.`requestNoteRedraw`().encodeToByteArray()
  }
}

object Adapter_a00aa705aa10bfc2e531 {
  @JvmStatic
  fun invoke(arg0: ByteArray, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    com.ziv.snfiletools.`getTotalPathTestState`(arg0.decodeToString(throwOnInvalidSequence = true)).encodeToByteArray()
  }
}

object Adapter_4f6ff35573aec0b49b51 {
  @JvmStatic
  fun invoke(arg0: ByteArray, arg1: ByteArray, completionToken: Long): kotlinx.coroutines.Job = SupernoteCoroutineBridge.launch(completionToken) {
    com.ziv.snfiletools.`setTotalPathTestState`(arg0.decodeToString(throwOnInvalidSequence = true), arg1.decodeToString(throwOnInvalidSequence = true)); null
  }
}

