#include "components_detail.hpp"
#include <charconv>
#include <cstdio>
#include <tuple>

namespace m3e {
using namespace detail;
namespace {

constexpr const char* Months[]{"January", "February", "March", "April", "May", "June",
                               "July", "August", "September", "October", "November", "December"};
constexpr Icon Back = 0xe5cb;
constexpr Icon CalendarIcon = 0xe916;

auto DateKey(Date date) { return std::make_tuple(date.year, date.month, date.day); }

int MonthDays(int year, int month) {
    constexpr int days[]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1] + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

bool Valid(Date d) {
    return d.year >= 1 && d.year <= 9999 && d.month >= 1 && d.month <= 12
        && d.day >= 1 && d.day <= MonthDays(d.year, d.month);
}

int FirstWeekday(CalendarPage page) {
    constexpr int offsets[]{0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    const int y = page.year - (page.month < 3);
    return (y + y / 4 - y / 100 + y / 400 + offsets[page.month - 1] + 1) % 7;
}

void MoveMonth(CalendarPage& page, int offset) {
    const int index = std::clamp((page.year - 1) * 12 + page.month - 1 + offset, 0, 9999 * 12 - 1);
    page = {index / 12 + 1, index % 12 + 1};
}

bool CalendarGrid(const Style& s, const char* id, CalendarPage& page, Date* date, DateRange* range) {
    if (SkipItems())
        return false;
    page.year = std::clamp(page.year, 1, 9999);
    page.month = std::clamp(page.month, 1, 12);
    const auto origin = ImGui::GetCursorScreenPos();
    const float cell = s.Px(40);
    Passive(At(origin, 7 * cell, s.Px(312)));
    ImGui::PushID(id);
    auto* draw = ImGui::GetWindowDrawList();
    const auto header = At(origin, 7 * cell, cell);
    const auto ink = s.theme[Role::OnSurface];
    auto arrow = [&](const char* key, float x, Icon icon, bool disabled) {
        const auto box = At({x, header.min.y}, cell, header.H());
        const auto item = Interact(key, box, disabled, false);
        State(s, draw, item, box, cell / 2, ink, true, true);
        Focus(s, draw, item, box, cell / 2, true);
        Symbol(s, draw, icon, s.Px(20), box.Center(), disabled ? Opacity(ink, .38f) : ink);
        return item.pressed;
    };
    if (arrow("Previous month", origin.x, Back, page.year == 1 && page.month == 1))
        MoveMonth(page, -1);
    if (arrow("Next month", header.max.x - cell, icons::ChevronRight, page.year == 9999 && page.month == 12))
        MoveMonth(page, 1);
    char caption[32];
    std::snprintf(caption, sizeof(caption), "%s %d", Months[page.month - 1], page.year);
    CenterText(s, draw, TitleSmall, header.Center(), ink, caption);
    constexpr const char* weekdays[]{"S", "M", "T", "W", "T", "F", "S"};
    for (int i = 0; i < 7; ++i) {
        CenterText(s, draw, LabelMedium, {origin.x + (i + .5f) * cell, origin.y + s.Px(56)},
                   s.theme[Role::OnSurfaceVariant], weekdays[i]);
    }
    bool changed = false;
    const int first = FirstWeekday(page);
    const int days = MonthDays(page.year, page.month);
    for (int day = 1; day <= days; ++day) {
        const Date d{page.year, page.month, day};
        const int i = first + day - 1;
        const auto box = At({origin.x + (i % 7) * cell, origin.y + s.Px(72) + (i / 7) * cell}, cell, cell);
        ImGui::PushID(day);
        const auto item = Interact("day", box, false, false);
        ImGui::PopID();
        if (item.pressed) {
            if (date) {
                changed = DateKey(*date) != DateKey(d);
                *date = d;
            } else if (!range->choosingEnd) {
                *range = {d, d, true};
                changed = true;
            } else {
                if (DateKey(d) < DateKey(range->start)) {
                    range->end = range->start;
                    range->start = d;
                } else {
                    range->end = d;
                }
                range->choosingEnd = false;
                changed = true;
            }
        }
        if (!item.visible)
            continue;
        const auto key = DateKey(d);
        const bool endpoint = date ? key == DateKey(*date)
                                   : key == DateKey(range->start) || (!range->choosingEnd && key == DateKey(range->end));
        const bool inRange = range && !range->choosingEnd && DateKey(range->start) < key && key < DateKey(range->end);
        const auto circle = box.Inset(s.Px(2));
        if (inRange)
            Fill(draw, box.Inset(s.Px(2)), 0, s.theme[Role::SecondaryContainer]);
        if (endpoint)
            Fill(draw, circle, cell / 2, s.theme[Role::Primary]);
        const auto color = endpoint ? s.theme[Role::OnPrimary] : ink;
        State(s, draw, item, circle, cell / 2, color, true, true);
        Focus(s, draw, item, circle, cell / 2, true);
        char dayText[4];
        std::snprintf(dayText, sizeof(dayText), "%d", day);
        CenterText(s, draw, BodyMedium, box.Center(), color, dayText);
    }
    ImGui::PopID();
    return changed;
}

bool ParseDate(std::string_view input, Date& date) {
    if (input.size() != 10 || input[4] != '-' || input[7] != '-')
        return false;
    const auto number = [&](int offset, int size, int& value) {
        const char* first = input.data() + offset;
        const auto result = std::from_chars(first, first + size, value);
        return result.ec == std::errc{} && result.ptr == first + size && first[0] >= '0' && first[0] <= '9';
    };
    return number(0, 4, date.year) && number(5, 2, date.month) && number(8, 2, date.day) && Valid(date);
}

void BeginPicker(const Style& s) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {s.Px(16), s.Px(16)});
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, s.Px(28));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, s.theme[Role::SurfaceContainerHigh]);
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
}

void EndPicker() {
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

bool DatePopup(const Style& s, const char* label, CalendarPage& page, Date* selected, DateRange* range) {
    if (SkipItems())
        return false;
    ImGui::PushID(label);
    const auto date = selected ? *selected : range->start;
    char value[96];
    if (range && !range->choosingEnd) {
        std::snprintf(value, sizeof(value), "%04d-%02d-%02d to %04d-%02d-%02d",
                      date.year, date.month, date.day, range->end.year, range->end.month, range->end.day);
    } else {
        std::snprintf(value, sizeof(value), "%04d-%02d-%02d", date.year, date.month, date.day);
    }
    const auto box = Slot(s, range ? 280 : 192, 56);
    if (Row(s, "date picker", box, {value, label, 0, CalendarIcon}, {Role::Count, Role::OnSurface, 4, 12, 0, true}).pressed) {
        if (Valid(date))
            page = {date.year, date.month};
        ImGui::OpenPopup("##calendar");
    }
    bool changed = false;
    BeginPicker(s);
    if (ImGui::BeginPopup("##calendar", ImGuiWindowFlags_NoSavedSettings)) {
        changed = CalendarGrid(s, "days", page, selected, range);
        if ((changed && (selected || !range->choosingEnd)) || Button(s, "Done", {Variant::Text}))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    EndPicker();
    ImGui::PopID();
    return changed;
}

} // namespace

bool DateInput(const Style& s, const char* label, Date& date, char* buffer, std::size_t capacity, float width) {
    if (!buffer || !capacity || SkipItems())
        return false;
    auto read = [&] { return std::string_view(buffer, std::find(buffer, buffer + capacity, '\0') - buffer); };
    auto* storage = ImGui::GetStateStorage();
    const auto initialized = StateKey(ImGui::GetID(label), "date initialized");
    if (!storage->GetBool(initialized)) {
        if (buffer[0] == '\0' && capacity >= 11 && Valid(date))
            std::snprintf(buffer, capacity, "%04d-%02d-%02d", date.year, date.month, date.day);
        storage->SetBool(initialized, true);
    }
    if (read().size() == capacity)
        return false;
    Date candidate;
    FieldOptions field;
    field.variant = FieldVariant::Outlined;
    field.width = width;
    field.placeholder = "YYYY-MM-DD";
    field.floatLabel = true;
    field.error = read().size() == 10 && !ParseDate(read(), candidate);
    field.supportingText = field.error ? "Enter a valid date (YYYY-MM-DD)" : nullptr;
    if (!TextField(s, label, buffer, capacity, field) || !ParseDate(read(), candidate))
        return false;
    const bool changed = DateKey(date) != DateKey(candidate);
    date = candidate;
    return changed;
}

bool Calendar(const Style& s, const char* id, CalendarPage& page, Date& date) {
    return CalendarGrid(s, id, page, &date, nullptr);
}
bool DateRangeCalendar(const Style& s, const char* id, CalendarPage& page, DateRange& range) {
    return CalendarGrid(s, id, page, nullptr, &range);
}
bool DatePicker(const Style& s, const char* label, CalendarPage& page, Date& date) {
    return DatePopup(s, label, page, &date, nullptr);
}
bool DateRangePicker(const Style& s, const char* label, CalendarPage& page, DateRange& range) {
    return DatePopup(s, label, page, nullptr, &range);
}

bool TimePicker(const Style& s, const char* label, Time& value, TimePickerMode mode, bool use24Hour) {
    if (SkipItems())
        return false;
    const int hour = std::clamp(value.hour, 0, 23), minute = std::clamp(value.minute, 0, 59);
    int nextHour = hour, nextMinute = minute;
    bool pm = hour >= 12;
    ImGui::PushID(label);
    Heading(s, label, Size::ExtraSmall);
    const auto store = ImGui::GetStateStorage();
    const auto minuteKey = ImGui::GetID("editing minute");
    bool pickMinute = store->GetBool(minuteKey);
    if (mode == TimePickerMode::Dial) {
        char hourLabel[16], minuteLabel[16];
        std::snprintf(hourLabel, sizeof(hourLabel), "%02d###hour", use24Hour ? hour : hour % 12 ? hour % 12 : 12);
        std::snprintf(minuteLabel, sizeof(minuteLabel), "%02d###minute", minute);
        if (Button(s, hourLabel, {pickMinute ? Variant::Outlined : Variant::Tonal, Size::Medium}))
            pickMinute = false;
        ImGui::SameLine();
        if (Button(s, minuteLabel, {pickMinute ? Variant::Tonal : Variant::Outlined, Size::Medium}))
            pickMinute = true;
        const auto box = Slot(s, 224, 224);
        Passive(box);
        auto* draw = ImGui::GetWindowDrawList();
        const auto center = box.Center();
        Fill(draw, box, box.W() / 2, s.theme[Role::SurfaceContainerHighest]);
        constexpr float Tau = 6.28318530718f;
        const int marks = !pickMinute && use24Hour ? 24 : 12;
        const int active = pickMinute ? (minute + 2) / 5 % 12 : use24Hour ? hour : hour % 12;
        for (int i = 0; i < marks; ++i) {
            const float theta = Tau * (i % 12) / 12;
            const auto r = s.Px(i < 12 ? 88 : 56);
            const auto point = ImVec2{center.x + r * std::sin(theta), center.y - r * std::cos(theta)};
            const auto target = Around(point, s.Px(i < 12 ? 38 : 28), s.Px(i < 12 ? 38 : 28));
            ImGui::PushID(i);
            const auto item = Interact("tick", target, false, false);
            ImGui::PopID();
            if (item.pressed) {
                if (pickMinute)
                    nextMinute = i * 5;
                else {
                    nextHour = use24Hour ? i : i + (pm ? 12 : 0);
                    store->SetBool(minuteKey, true);
                }
            }
            if (i == active) {
                draw->AddLine(center, point, s.theme[Role::Primary], s.Px(2));
                Fill(draw, target, target.W() / 2, s.theme[Role::Primary]);
            }
            const auto ink = s.theme[i == active ? Role::OnPrimary : Role::OnSurface];
            State(s, draw, item, target, target.W() / 2, ink, true, true);
            Focus(s, draw, item, target, target.W() / 2, true);
            char digits[4];
            std::snprintf(digits, sizeof(digits), "%02d", pickMinute ? i * 5 : i == 0 && !use24Hour ? 12 : i);
            CenterText(s, draw, BodyMedium, point, ink, digits);
        }
        if (pickMinute || nextHour == hour)
            store->SetBool(minuteKey, pickMinute);
    } else {
        int displayHour = use24Hour ? hour : hour % 12 ? hour % 12 : 12;
        const int minHour = use24Hour ? 0 : 1, maxHour = use24Hour ? 23 : 12;
        ImGui::PushStyleColor(ImGuiCol_FrameBg, s.theme[Role::SurfaceContainerHighest]);
        ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
        ImGui::SetNextItemWidth(s.Px(80));
        if (mode == TimePickerMode::Input)
            ImGui::InputInt("##hour", &displayHour, 0);
        else
            ImGui::DragInt("##hour", &displayHour, .25f, minHour, maxHour, "%02d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(s.Px(80));
        if (mode == TimePickerMode::Input)
            ImGui::InputInt("##minute", &nextMinute, 0);
        else
            ImGui::DragInt("##minute", &nextMinute, .25f, 0, 59, "%02d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopStyleColor(2);
        displayHour = std::clamp(displayHour, minHour, maxHour);
        nextHour = use24Hour ? displayHour : displayHour % 12 + (pm ? 12 : 0);
        nextMinute = std::clamp(nextMinute, 0, 59);
    }
    if (!use24Hour) {
        const Choice periods[]{{"AM"}, {"PM"}};
        int period = pm ? 1 : 0;
        if (SegmentedButtons(s, "Period", periods, 2, period, false))
            nextHour = nextHour % 12 + period * 12;
    }
    ImGui::PopID();
    const bool changed = nextHour != value.hour || nextMinute != value.minute;
    value = {nextHour, nextMinute};
    return changed;
}

} // namespace m3e
