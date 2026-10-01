package com.ziv.snfiletools

import supernote.generated.annotations.SupernotePluginExport

@SupernotePluginExport
fun greetFromJvm(name: String): String = "Hello, $name"
