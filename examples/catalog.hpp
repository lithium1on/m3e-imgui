#pragma once

#include <m3e/components.hpp>

struct CatalogState {
    char query[3][96]{}, notes[256]{"First line\nAdd another line to grow the field."}, dateText[16]{};
    int selected = 0, segmented = 1, radio = 0, list = 0, navigation = 0, rail = 0, toc = 0;
    int page = 0, step = 0, tree = 1, slide = 0, carousel = 0;
    bool segments[3]{true, false, true}, branches[6]{true, true, false, false, false, false};
    bool filter = true, token = true, expansion = true, drawer = true;
    bool modal = false, persistent = false, dismissed = false, drawerModal = false, pulling = false;
    float split = .5f, vertical = .5f, collapse = .25f, morph = .5f;
    double refreshedUntil = 0;
    m3e::CalendarPage months[3]{{2026, 11}, {2026, 11}, {2026, 11}};
    m3e::Date dates[3]{{2026, 11, 21}, {2026, 11, 21}};
    m3e::DateRange range{{2026, 11, 18}, {2026, 11, 24}};
    m3e::Time times[3]{{9, 15}, {13, 30}, {16, 0}};
    m3e::SnackbarState snackbar;
};

void ShowCatalog(const m3e::Style&, ImVec2 origin, CatalogState&, int page);
