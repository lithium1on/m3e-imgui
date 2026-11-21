plugins {
    id("com.android.application") version "9.5.2"
    id("org.jetbrains.kotlin.android") version "2.4.10" apply false
    id("org.jetbrains.kotlin.plugin.compose") version "2.4.10"
}

abstract class PrepareFonts : DefaultTask() {
    @get:InputFiles abstract val fonts: ConfigurableFileCollection
    @get:InputFiles abstract val notices: ConfigurableFileCollection
    @get:OutputDirectory abstract val resources: DirectoryProperty
    @get:OutputDirectory abstract val assets: DirectoryProperty

    @TaskAction
    fun writeFonts() {
        val fontDirectory = resources.dir("font").get().asFile
        fontDirectory.mkdirs()
        val resourceNames = mapOf(
            "MaterialSymbolsOutlined.ttf" to "material_symbols.ttf",
            "MaterialSymbolsOutlined-Filled.ttf" to "material_symbols_filled.ttf",
            "GoogleSansFlex-Regular.ttf" to "google_sans_flex_regular.ttf",
            "GoogleSansFlex-Medium.ttf" to "google_sans_flex_medium.ttf",
        )
        for (font in fonts) {
            font.copyTo(fontDirectory.resolve(resourceNames.getValue(font.name)), overwrite = true)
        }
        val licenseDirectory = assets.dir("licenses").get().asFile
        licenseDirectory.mkdirs()
        for (notice in notices) {
            notice.copyTo(licenseDirectory.resolve(notice.name), overwrite = true)
        }
    }
}

val prepareFonts = tasks.register<PrepareFonts>("prepareFonts") {
    fonts.from(fileTree("../../assets/fonts") {
        include("MaterialSymbolsOutlined*.ttf", "GoogleSansFlex-*.ttf")
    })
    resources.set(layout.buildDirectory.dir("generated/font-resources"))
    notices.from(fileTree("../../licenses") {
        include("GoogleSansFlex-OFL.txt", "MaterialSymbolsOutlined-Apache-2.0.txt")
    })
    assets.set(layout.buildDirectory.dir("generated/font-assets"))
}

android {
    namespace = "dev.m3e.reference"
    compileSdk = 37
    buildToolsVersion = "37.0.0"

    defaultConfig {
        applicationId = "dev.m3e.reference"
        minSdk = 31
        targetSdk = 36
    }

    buildFeatures { compose = true }
}

androidComponents.onVariants { variant ->
    variant.sources.res?.addGeneratedSourceDirectory(prepareFonts, PrepareFonts::resources)
    variant.sources.assets?.addGeneratedSourceDirectory(prepareFonts, PrepareFonts::assets)
}

dependencies {
    implementation("androidx.activity:activity-compose:1.13.0")
    implementation("androidx.compose.material3:material3:1.5.0")
}
