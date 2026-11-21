package dev.m3e.reference

import android.util.Log
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

private fun sampleEvent(event: String) = Log.i("M3EReference", event)

@OptIn(ExperimentalMaterial3Api::class, ExperimentalMaterial3ExpressiveApi::class)
@Composable
internal fun CatalogReference() {
    var filter by remember { mutableStateOf(true) }
    var segment by remember { mutableIntStateOf(1) }
    var sheet by remember { mutableStateOf(false) }
    var snackbar by remember { mutableStateOf(false) }
    val labels = listOf("Today", "Week", "Month")
    val actions = listOf("Search" to 0xe8b6, "Settings" to 0xe8b8, "Save" to 0xe5ca)
    val colors = MaterialTheme.colorScheme
    fun at(x: Int, y: Int) = Modifier.offset(x = x.dp, y = y.dp)
    Box {
        Box(at(16, 16).size(width = 96.dp, height = 48.dp), contentAlignment = Alignment.CenterStart) {
            AssistChip(onClick = {}, label = { Text("Assist") }, leadingIcon = { Symbol(0xe88e, size = 18) })
        }
        Box(at(116, 16).size(width = 100.dp, height = 48.dp), contentAlignment = Alignment.CenterStart) {
            FilterChip(selected = filter, onClick = {
                filter = !filter
                sampleEvent("filter=$filter")
            }, label = { Text("Filter") }, leadingIcon = if (filter) { { Symbol(0xe5ca, size = 18) } } else null)
        }
        Box(at(220, 16).size(width = 110.dp, height = 48.dp), contentAlignment = Alignment.CenterStart) {
            SuggestionChip(onClick = {}, label = { Text("Suggest") })
        }
        SingleChoiceSegmentedButtonRow(at(16, 88)) {
            labels.forEachIndexed { index, title ->
                SegmentedButton(
                    selected = segment == index, onClick = {
                        segment = index
                        sampleEvent("segment=$index")
                    },
                    shape = SegmentedButtonDefaults.itemShape(index, labels.size),
                    label = { Text(title) }
                )
            }
        }
        Card(at(16, 154).size(156.dp, 104.dp)) {
            Column(Modifier.padding(16.dp)) {
                Text("Notebook", fontSize = 20.sp)
                Text("Filled card")
            }
        }
        OutlinedCard(at(188, 154).size(156.dp, 104.dp)) {
            Column(Modifier.padding(16.dp)) {
                Text("Library", fontSize = 20.sp)
                Text("Outlined card")
            }
        }
        ListItem(
            headlineContent = { Text("Today's notes") },
            supportingContent = { Text("Two items edited today") },
            leadingContent = { Symbol(0xe3c9, "Edit") },
            trailingContent = { Symbol(0xe5cc, "Next") },
            colors = ListItemDefaults.colors(containerColor = colors.surfaceContainerLowest),
            modifier = at(16, 282).width(328.dp)
        )
        RadioButton(selected = true, onClick = {}, modifier = at(16, 380))
        Text("Default", at(64, 391))
        Badge(at(160, 396)) { Text("12") }
        ContainedLoadingIndicator(progress = { .62f }, modifier = at(246, 380))
        Button(onClick = {
            sheet = true
            sampleEvent("sheet=true")
        }, modifier = at(16, 460)) { Text("Open sheet") }
        FilledTonalButton(onClick = {
            snackbar = true
            sampleEvent("snackbar=true")
        }, modifier = at(174, 460)) { Text("Notify") }
        BottomAppBar(modifier = at(16, 568).width(328.dp), windowInsets = WindowInsets(0, 0, 0, 0), actions = {
            for ((title, icon) in actions)
                IconButton({ sampleEvent("bottom=$title") }) { Symbol(icon, title) }
        })
        if (sheet) {
            ModalBottomSheet(onDismissRequest = { sheet = false }) {
                Column(Modifier.padding(horizontal = 24.dp).padding(bottom = 32.dp)) {
                    Text("Note details", style = MaterialTheme.typography.headlineSmall)
                    TextButton(onClick = { sheet = false }) { Text("Close") }
                }
            }
        }
        if (snackbar) {
            Snackbar(modifier = at(16, 630).widthIn(max = 328.dp), action = {
                TextButton(onClick = {
                    snackbar = false
                    sampleEvent("snackbar=undo")
                }) { Text("Undo") }
            }) {
                Text("Action completed")
            }
        }
    }
}
