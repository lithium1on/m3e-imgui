export function drawCatalog(name, place, icon) {
  const heading = (x, y, label) => place(x, y, `<m3e-heading variant="title" size="large">${label}</m3e-heading>`);
  const row = (body, gap = 8, alignment = "center") => `<div style="display:flex;align-items:${alignment};gap:${gap}px">${body}</div>`;
  const files = ["Notes", "Library", "Saved", "Archive", "Shared"];
  const segments = ["Today", "Week", "Month"];
  const actions = [["Search", "search"], ["Settings", "settings"], ["Save", "check"]];
  const actionButtons = (variant = "text") => actions.map(([label, glyph]) =>
    `<m3e-button variant="${variant}">${icon(glyph, "icon")}${label}</m3e-button>`).join("");
  const first = (x, y, label, value) => {
    heading(x, y, label);
    place(x, y + 36, value);
  };

  if (name === "selection") {
    first(24, 16, "Chips", row(`
      <m3e-assist-chip>${icon("info", "icon")}Assist</m3e-assist-chip>
      <m3e-filter-chip-set multi><m3e-filter-chip selected>Filter</m3e-filter-chip></m3e-filter-chip-set>
      <m3e-suggestion-chip variant="elevated">Suggestion</m3e-suggestion-chip>
      <m3e-assist-chip disabled>${icon("info", "icon")}Disabled</m3e-assist-chip>`));
    place(24, 118, `<m3e-input-chip>${icon("favorite", "icon")}Saved tag${icon("close", "remove")}</m3e-input-chip>`);
    heading(24, 185, "Radio group");
    place(24, 222, `<m3e-radio-group style="display:flex;gap:8px">${files.map((label, i) =>
      `<label style="display:flex;align-items:center"><m3e-radio value="${i}" ${i === 0 ? "checked" : ""}
       ${i === 4 ? "disabled" : ""}></m3e-radio>${label}</label>`).join("")}</m3e-radio-group>`);
    const group = (checks, multi) => `<m3e-segmented-button ${multi ? "multi hide-selection-indicator" : ""}>
      ${segments.map((label, i) => `<m3e-button-segment ${checks.includes(i) ? "checked" : ""}>${label}</m3e-button-segment>`).join("")}
      </m3e-segmented-button>`;
    first(24, 306, "Single selection", group([1], false));
    first(24, 414, "Multiple selection", group([0, 2], true));
    first(24, 530, "Standard button group", `<m3e-button-group>${actionButtons("filled")}</m3e-button-group>`);
  }

  if (name === "search-inputs") {
    const options = files.map((label, i) =>
      `<m3e-option ${i === 4 ? "disabled" : ""} value="${i}">${label}</m3e-option>`).join("");
    first(24, 16, "Autocomplete", `<m3e-form-field variant="outlined" hide-subscript="auto" style="width:320px">
        <input id="autocomplete" placeholder="Find a destination">
      </m3e-form-field><m3e-autocomplete for="autocomplete">${options}</m3e-autocomplete>`);
    const search = (placeholder, width, fullscreen = false) =>
      `<m3e-search-view style="width:${width}px" ${fullscreen ? 'mode="fullscreen"' : ""}>
        ${icon("search", "leading")}<input slot="input" placeholder="${placeholder}">
        <m3e-list>${files.map(label => `<m3e-list-item>${label}</m3e-list-item>`).join("")}</m3e-list>
      </m3e-search-view>`;
    first(382, 16, "Docked search", search("Search notes", 320));
    first(24, 288, "Select", `<m3e-form-field variant="outlined" style="width:240px" hide-subscript="auto">
      <m3e-select value="0">${options}</m3e-select></m3e-form-field>`);
    first(382, 288, "Auto-growing textarea", `<m3e-form-field variant="filled" style="width:300px" hide-subscript="auto">
      <label slot="label">Notes</label><textarea id="notes">First line\nAdd another line to grow the field.</textarea>
      </m3e-form-field><m3e-textarea-autosize for="notes"></m3e-textarea-autosize>`);
    first(24, 500, "Expanded search", search("Search your library", 712, true));
  }

  if (name === "date-time") {
    first(24, 16, "Calendar", '<m3e-calendar date="2026-11-21" start-at="2026-11-21"></m3e-calendar>');
    heading(382, 16, "Date and date-range pickers");
    const dateField = (id, value, label) => `<m3e-form-field variant="outlined" hide-subscript="auto">
      <label slot="label" for="${id}">${label}</label><m3e-date-input id="${id}" value="${value}"></m3e-date-input>
      <m3e-icon-button slot="suffix">${icon("calendar_today")}<m3e-datepicker-toggle for="${id}-picker"></m3e-datepicker-toggle></m3e-icon-button>
      </m3e-form-field><m3e-datepicker id="${id}-picker" for="${id}"></m3e-datepicker>`;
    place(382, 52, dateField("single", "2026-11-21", "Pick a date"));
    place(382, 130, '<m3e-calendar range-start="2026-11-18" range-end="2026-11-24" start-at="2026-11-21"></m3e-calendar>');
    place(382, 454, `<m3e-form-field variant="outlined"><label slot="label" for="date-entry">Type a date</label>
      <m3e-date-input id="date-entry" value="2026-11-21"></m3e-date-input></m3e-form-field>`);
    first(24, 380, "Clock dial", '<m3e-timepicker-dial></m3e-timepicker-dial>');
    first(382, 542, "Time input (24-hour)", '<m3e-timepicker-input></m3e-timepicker-input>');
  }

  if (name === "cards-lists") {
    heading(24, 16, "Cards");
    ["Filled", "Outlined", "Elevated"].forEach((label, i) =>
      place(24 + 230 * i, 52, `<m3e-card variant="${label.toLowerCase()}" style="width:208px;min-height:128px;box-sizing:border-box">
        <m3e-heading slot="header" variant="title" size="large">${label}</m3e-heading>
        Notebook preview<m3e-button slot="actions">Open</m3e-button></m3e-card>`));
    place(24, 220, `<m3e-content-pane style="width:324px;min-height:200px;box-sizing:border-box">
      <m3e-heading variant="title" size="large">Content pane</m3e-heading><m3e-divider></m3e-divider>
      <m3e-expansion-panel open><span slot="header">${icon("info")} Details</span>
        The expansion panel shows additional information.</m3e-expansion-panel></m3e-content-pane>`);
    first(382, 218, "Segmented list", `<m3e-list variant="segmented" style="width:320px">
      <m3e-list-item>${icon("edit", "leading")}Today's notes<span slot="supporting-text">Two items edited today</span>${icon("chevron_right", "trailing")}</m3e-list-item>
      <m3e-list-item>${icon("favorite", "leading")}Favorites<span slot="supporting-text">Pinned to your library</span>${icon("chevron_right", "trailing")}</m3e-list-item>
      <m3e-list-item>${icon("archive", "leading")}Archived</m3e-list-item></m3e-list>`);
    place(24, 476, '<label style="display:flex;align-items:center;gap:8px"><m3e-checkbox checked></m3e-checkbox>Show drawer</label>');
    place(24, 521, `<m3e-drawer-container start style="width:712px;height:230px;background:var(--md-sys-color-surface-container)">
      <m3e-nav-menu slot="start" style="width:224px">
        ${files.slice(0, 3).map((file, i) => `<m3e-nav-menu-item>${icon(["edit", "home", "favorite"][i], "icon")}${file}</m3e-nav-menu-item>`).join("")}
      </m3e-nav-menu><div style="padding:16px"><m3e-heading variant="title" size="large">Notes</m3e-heading><p>Drawer content</p></div>
      </m3e-drawer-container>`);
  }

  if (name === "navigation-layout") {
    first(24, 16, "Navigation rail", `<m3e-nav-rail mode="compact" style="height:215px">
      ${files.slice(0, 3).map((file, i) => `<m3e-nav-item ${i === 0 ? "selected" : ""}>
        ${icon(["edit", "home", "favorite"][i], "icon")}${file}</m3e-nav-item>`).join("")}</m3e-nav-rail>`);
    first(154, 16, "Table of contents", `<div id="contents">
      <m3e-toc for="contents"></m3e-toc><div style="position:absolute;left:-1000px">
        <h2 id="c-notes">Notes</h2><h2 id="c-library">Library</h2><h2 id="c-saved">Saved</h2></div></div>`);
    first(382, 16, "Tree", `<m3e-tree style="width:240px"><m3e-tree-item open>Library
      <m3e-tree-item open selected>Notes<m3e-tree-item>Drafts</m3e-tree-item><m3e-tree-item>Recipes</m3e-tree-item></m3e-tree-item>
      <m3e-tree-item>Saved</m3e-tree-item></m3e-tree-item><m3e-tree-item disabled>Shared</m3e-tree-item></m3e-tree>`);
    heading(24, 319, "Breadcrumbs");
    place(24, 352, `<m3e-breadcrumb>${files.slice(0, 3).map(label => `<m3e-breadcrumb-item>${label}</m3e-breadcrumb-item>`).join("")}</m3e-breadcrumb>`);
    first(24, 440, "Paginator", '<m3e-paginator length="80" page-size="10"></m3e-paginator>');
    first(382, 440, "Stepper", `<m3e-stepper>${segments.map((label, i) => `<m3e-step for="step${i}">${label}</m3e-step>`).join("")}
      ${segments.map((_, i) => `<m3e-step-panel id="step${i}"></m3e-step-panel>`).join("")}</m3e-stepper>`);
  }

  if (name === "panes-carousel") {
    const pane = (slot, label, background) => `<div slot="${slot}" style="height:100%;border-radius:12px;padding:16px;box-sizing:border-box;
      background:var(--md-sys-color-${background})"><m3e-heading variant="title" size="large">${label}</m3e-heading></div>`;
    const split = (a, b, vertical, width, height) =>
      `<m3e-split-pane value="50" ${vertical ? 'orientation="vertical"' : ""} style="width:${width}px;height:${height}px">
        ${pane("start", a, "surface-container-low")}${pane("end", b, "surface-container")}
      </m3e-split-pane>`;
    first(24, 16, "Split panes", split("Left", "Right", false, 680, 160));
    place(24, 246, split("Top", "Bottom", true, 304, 176));
    place(382, 246, `<m3e-carousel variant="hero" style="width:316px;height:176px">${segments.map(label =>
      `<m3e-carousel-item><m3e-card><m3e-heading variant="title" size="large">${label}</m3e-heading>
        Drag to change slides.</m3e-card></m3e-carousel-item>`).join("")}</m3e-carousel>`);
    heading(24, 458, "Slide group");
    place(24, 490, `<m3e-slide-group style="width:290px">${files.map(label => `<m3e-chip>${label}</m3e-chip>`).join("")}</m3e-slide-group>`);
  }

  if (name === "feedback-shapes") {
    first(24, 16, "Avatar and badge", row(`<m3e-avatar>AB</m3e-avatar>
      <m3e-avatar>${icon("person")}</m3e-avatar><m3e-badge>12</m3e-badge><m3e-badge size="small"></m3e-badge>`));
    const callout = (x, y, title, message, glyph, variant) => place(x, y,
      `<m3e-callout variant="${variant}" style="width:320px;min-height:72px;box-sizing:border-box">
        ${icon(glyph, "icon")}<span slot="label">${title}</span>${message}</m3e-callout>`);
    callout(24, 130, "Information", "Notes sync across your devices.", "info", "secondary");
    callout(382, 130, "Saved", "Your changes are up to date.", "check", "primary");
    callout(24, 218, "Offline", "These changes will sync later.", "warning", "tertiary");
    callout(382, 218, "Could not sync", "Check your connection.", "error", "error");
    heading(24, 328, "Shapes");
    const shapes = ["circle","square","pill","diamond","4-leaf-clover","burst","heart","4-sided-cookie","6-sided-cookie",
      "7-sided-cookie","8-leaf-clover","9-sided-cookie","12-sided-cookie","arch","arrow","boom","bun","fan","flower","gem",
      "ghost-ish","hexagon","oval","pentagon","pixel-circle","pixel-triangle","puffy","puffy-diamond","semicircle","slanted",
      "soft-boom","soft-burst","sunny","triangle","very-sunny"];
    shapes.forEach((shape, i) => place(24 + (i % 12) * 55, 364 + Math.floor(i / 12) * 55,
      `<m3e-shape name="${shape}" style="--m3e-shape-size:42px"></m3e-shape>`));
    first(24, 548, "Loading indicators", row(`<m3e-loading-indicator></m3e-loading-indicator>
      <m3e-loading-indicator variant="contained" progress="0.65"></m3e-loading-indicator>`));
    first(382, 548, "Skeleton", row(`<m3e-skeleton shape="circular" style="width:44px;height:44px"></m3e-skeleton>
      <div><m3e-skeleton style="width:180px;height:14px"></m3e-skeleton>
        <m3e-skeleton style="width:120px;height:10px;margin-top:10px"></m3e-skeleton></div>`));
    place(24, 654, '<m3e-divider style="width:678px"></m3e-divider>');
    heading(24, 674, "Shape morph");
    place(24, 710, '<m3e-slider style="width:240px"><m3e-slider-thumb value="50"></m3e-slider-thumb></m3e-slider>');
    place(290, 709, '<m3e-shape name="soft-burst"></m3e-shape>');
  }

  if (name === "sheets-actions") {
    first(24, 16, "Sheets", row(`<m3e-button variant="filled"><m3e-bottom-sheet-trigger for="modal-sheet">Open modal sheet</m3e-bottom-sheet-trigger></m3e-button>
      <m3e-button variant="tonal"><m3e-bottom-sheet-trigger for="persistent-sheet">Toggle persistent sheet</m3e-bottom-sheet-trigger></m3e-button>`));
    const items = files.slice(0, 3).map(label => `<m3e-list-action>${label}</m3e-list-action>`).join("");
    place(24, 105, `<m3e-bottom-sheet id="modal-sheet">
      <m3e-heading variant="title" size="large">Move note</m3e-heading>
      <p>Choose a destination, or drag the handle down to close.</p><m3e-action-list>${items}</m3e-action-list>
      </m3e-bottom-sheet><m3e-bottom-sheet id="persistent-sheet" mode="standard">Note details</m3e-bottom-sheet>`);
    first(24, 139, "Split button and FAB menu", row(`<m3e-split-button variant="filled"><m3e-button slot="leading-button">Save</m3e-button>
      <m3e-icon-button slot="trailing-button">${icon("expand_more")}<m3e-menu-trigger for="split-actions"></m3e-menu-trigger>
      </m3e-icon-button></m3e-split-button><m3e-menu id="split-actions">${actions.map(([label, glyph]) =>
      `<m3e-menu-item>${icon(glyph, "icon")}${label}</m3e-menu-item>`).join("")}</m3e-menu>
      <m3e-fab aria-label="Create"><m3e-fab-menu-trigger for="fab-actions">${icon("add")}</m3e-fab-menu-trigger></m3e-fab>
      <m3e-fab-menu id="fab-actions">${actions.map(([label, glyph]) =>
      `<m3e-fab-menu-item>${icon(glyph, "icon")}${label}</m3e-fab-menu-item>`).join("")}</m3e-fab-menu>`, 32));
    heading(24, 272, "Toolbar");
    place(24, 307, `<m3e-toolbar>${actionButtons()}</m3e-toolbar>`);
    place(24, 391, `<m3e-toolbar variant="vibrant" shape="rounded" elevated>${actionButtons()}</m3e-toolbar>`);
    place(382, 307, `<m3e-toolbar shape="rounded" vertical>${actions.map(([, glyph]) =>
      `<m3e-icon-button>${icon(glyph)}</m3e-icon-button>`).join("")}</m3e-toolbar>`);
    place(24, 510, '<m3e-button id="notify" variant="tonal">Show snackbar</m3e-button>');
    place(24, 566, '<m3e-snackbar id="notice" dismissible>Saved to your library</m3e-snackbar>');
    document.querySelector("#notify").addEventListener("click", () => { document.querySelector("#notice").open = true; });
  }
}
