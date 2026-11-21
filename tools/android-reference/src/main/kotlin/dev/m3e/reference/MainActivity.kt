package dev.m3e.reference

import android.os.Bundle
import android.util.Log
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.CircularWavyProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExperimentalMaterial3ExpressiveApi
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.FloatingToolbarDefaults
import androidx.compose.material3.HorizontalFloatingToolbar
import androidx.compose.material3.IconButton
import androidx.compose.material3.IconButtonDefaults
import androidx.compose.material3.IconToggleButton
import androidx.compose.material3.LargeFloatingActionButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.LinearWavyProgressIndicator
import androidx.compose.material3.MaterialExpressiveTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.MediumFloatingActionButton
import androidx.compose.material3.MotionScheme
import androidx.compose.material3.NavigationItemIconPosition
import androidx.compose.material3.ShortNavigationBar
import androidx.compose.material3.ShortNavigationBarItem
import androidx.compose.material3.Slider
import androidx.compose.material3.SmallExtendedFloatingActionButton
import androidx.compose.material3.SmallFloatingActionButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.Typography
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.expressiveLightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.boundsInWindow
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.PlatformTextStyle
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import kotlin.math.roundToInt

private const val Tag = "M3EReference"
private val destinations = listOf("Library" to 0xe88a, "Explore" to 0xe8b6, "Saved" to 0xe87d)
private val symbols = FontFamily(Font(R.font.material_symbols))
private val filledSymbols = FontFamily(Font(R.font.material_symbols_filled))
private val googleSansFlex = FontFamily(
    Font(R.font.google_sans_flex_regular, FontWeight.Normal),
    Font(R.font.google_sans_flex_medium, FontWeight.Medium),
)
private val defaults = Typography()
private fun TextStyle.withGoogleSansFlex() = copy(fontFamily = googleSansFlex)
private val typography = Typography(
    displayLarge = defaults.displayLarge.withGoogleSansFlex(),
    displayMedium = defaults.displayMedium.withGoogleSansFlex(),
    displaySmall = defaults.displaySmall.withGoogleSansFlex(),
    headlineLarge = defaults.headlineLarge.withGoogleSansFlex(),
    headlineMedium = defaults.headlineMedium.withGoogleSansFlex(),
    headlineSmall = defaults.headlineSmall.withGoogleSansFlex(),
    titleLarge = defaults.titleLarge.withGoogleSansFlex(),
    titleMedium = defaults.titleMedium.withGoogleSansFlex(),
    titleSmall = defaults.titleSmall.withGoogleSansFlex(),
    bodyLarge = defaults.bodyLarge.withGoogleSansFlex(),
    bodyMedium = defaults.bodyMedium.withGoogleSansFlex(),
    bodySmall = defaults.bodySmall.withGoogleSansFlex(),
    labelLarge = defaults.labelLarge.withGoogleSansFlex(),
    labelMedium = defaults.labelMedium.withGoogleSansFlex(),
    labelSmall = defaults.labelSmall.withGoogleSansFlex(),
)

class MainActivity : ComponentActivity() {
    @OptIn(ExperimentalMaterial3ExpressiveApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        if (intent.getBooleanExtra("capture", false)) {
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        }
        WindowInsetsControllerCompat(window, window.decorView).apply {
            systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            hide(WindowInsetsCompat.Type.systemBars())
        }
        val palette = intent.getStringExtra("palette") ?: "light"
        val fixture = intent.getStringExtra("fixture")
        setContent {
            val colors = when (palette) {
                "dark" -> darkColorScheme()
                "dynamic-light" -> dynamicLightColorScheme(this)
                "dynamic-dark" -> dynamicDarkColorScheme(this)
                else -> expressiveLightColorScheme()
            }
            SideEffect {
                Log.i(Tag, "palette=$palette primary=${colors.primary.toArgb()} container=${colors.primaryContainer.toArgb()} " +
                    "fontScale=${resources.configuration.fontScale}")
            }
            MaterialExpressiveTheme(colorScheme = colors, typography = typography, motionScheme = MotionScheme.expressive()) {
                Box(Modifier.fillMaxSize().background(colors.surfaceDim), contentAlignment = Alignment.TopCenter) {
                    Surface(
                        modifier = Modifier.offset(y = 20.dp)
                            .size(360.dp, 688.dp)
                            .onGloballyPositioned {
                                val r = it.boundsInWindow()
                                Log.i(Tag, "captureBounds=${r.left.roundToInt()},${r.top.roundToInt()}," +
                                    "${r.right.roundToInt()},${r.bottom.roundToInt()}")
                            },
                        color = colors.surfaceContainerLowest,
                        contentColor = colors.onSurface,
                    ) {
                        if (fixture == "catalog") CatalogReference() else Reference()
                    }
                }
            }
        }
    }
}

@Composable
internal fun Symbol(code: Int, label: String? = null, size: Int = 24, filled: Boolean = false) {
    Text(
        text = code.toChar().toString(),
        modifier = if (label == null) Modifier else Modifier.semantics { contentDescription = label },
        style = TextStyle(
            fontFamily = if (filled) filledSymbols else symbols,
            fontSize = with(LocalDensity.current) { size.dp.toSp() },
            platformStyle = PlatformTextStyle(includeFontPadding = false)
        )
    )
}

@OptIn(ExperimentalMaterial3Api::class, ExperimentalMaterial3ExpressiveApi::class)
@Composable
private fun Reference() {
    var destination by remember { mutableIntStateOf(0) }
    var notes by remember { mutableIntStateOf(0) }
    var saved by remember { mutableStateOf(true) }
    var slider by remember { mutableFloatStateOf(.62f) }
    var nearEndSlider by remember { mutableFloatStateOf(.98f) }

    fun navigate(index: Int) {
        destination = index
        Log.i(Tag, "destination=${destinations[index].first}")
    }

    fun addNote() {
        notes++
        Log.i(Tag, "notes=$notes")
    }

    fun at(x: Int, y: Int) = Modifier.offset(x = x.dp, y = y.dp)

    Box {
        TopAppBar(
            modifier = at(16, 16).width(328.dp),
            title = { Text(destinations[destination].first) },
            navigationIcon = { IconButton({ navigate(0) }) { Symbol(0xe5c4, "Back") } },
            actions = { IconButton({ navigate(1) }) { Symbol(0xe8b6, "Search") } },
            windowInsets = WindowInsets(0, 0, 0, 0)
        )
        // Give the bar a bounded maximum width without forcing each item to that width.
        Box(at(16, 104).width(328.dp)) {
            ShortNavigationBar(windowInsets = WindowInsets(0, 0, 0, 0)) {
                for ((index, item) in destinations.withIndex()) {
                    ShortNavigationBarItem(
                        selected = index == destination,
                        onClick = { navigate(index) },
                        icon = { Symbol(item.second, filled = index == destination) },
                        label = { Text(item.first) },
                        iconPosition = NavigationItemIconPosition.Start
                    )
                }
            }
        }
        val colors = MaterialTheme.colorScheme
        HorizontalFloatingToolbar(
            expanded = true,
            modifier = at(16, 192),
            colors = FloatingToolbarDefaults.vibrantFloatingToolbarColors()
        ) {
            IconToggleButton(checked = saved, onCheckedChange = {
                saved = it
                Log.i(Tag, "favorite=$saved")
            }, colors = IconButtonDefaults.iconToggleButtonColors(
                containerColor = Color.Transparent,
                contentColor = colors.onPrimaryContainer,
                checkedContainerColor = colors.surfaceContainer,
                checkedContentColor = colors.onSurface
            )) {
                Symbol(0xe87d, "Favorite", filled = saved)
            }
            IconButton({ navigate(1) }) { Symbol(0xe8b8, "Explore options") }
            IconButton(::addNote) { Symbol(0xe3c9, "Edit") }
        }
        Box(at(16, 284).size(48.dp), contentAlignment = Alignment.Center) {
            SmallFloatingActionButton(::addNote) { Symbol(0xe145, "Small add") }
        }
        FloatingActionButton(::addNote, at(80, 280), containerColor = colors.secondaryContainer) { Symbol(0xe145, "Add") }
        MediumFloatingActionButton(::addNote, at(152, 268), containerColor = colors.tertiaryContainer) {
            Symbol(0xe145, "Medium add", 28)
        }
        LargeFloatingActionButton(::addNote, at(248, 260), containerColor = colors.surfaceContainerHigh,
            contentColor = colors.primary) { Symbol(0xe145, "Large add", 36) }
        SmallExtendedFloatingActionButton(text = { Text("New note") }, icon = { Symbol(0xe3c9, "New note") },
            onClick = ::addNote, modifier = at(16, 380))
        if (notes > 0) Text("Draft notes: $notes", at(175, 399))
        LinearProgressIndicator(progress = { .62f }, modifier = at(16, 482).width(240.dp))
        LinearWavyProgressIndicator(progress = { .62f }, modifier = at(16, 514).width(240.dp), waveSpeed = 0.dp)
        CircularProgressIndicator(progress = { .62f }, modifier = at(278, 478))
        CircularWavyProgressIndicator(progress = { .62f }, modifier = at(274, 520), waveSpeed = 0.dp)
        Slider(value = slider, onValueChange = { slider = it }, modifier = at(16, 580).width(240.dp))
        Text("${(slider * 100).roundToInt()}%", at(268, 592))
        Slider(value = nearEndSlider, onValueChange = {
            nearEndSlider = it
            Log.i(Tag, "slider=${(it * 100).roundToInt()}")
        }, modifier = at(16, 636).width(240.dp))
        Text("${(nearEndSlider * 100).roundToInt()}%", at(268, 648))
    }
}
