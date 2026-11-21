import "./node_modules/@m3e/web/dist/all.min.js";
import { drawCatalog } from "./catalog-reference.js";

const query = new URLSearchParams(location.search);
const sample = query.get("case") || "button";
const theme = document.querySelector("m3e-theme");
theme.scheme = query.get("scheme") || "light";
theme.density = Number(query.get("density") || 0);
const main = document.querySelector("main");
function place(x, y, content, style = "") {
  main.insertAdjacentHTML("beforeend", `<div class="sample" style="left:${x}px;top:${y}px;${style}">${content}</div>`);
}

function icon(name, slot = "") {
  return `<m3e-icon ${slot ? 'slot="' + slot + '"' : ""} name="${name}"></m3e-icon>`;
}

const variants = ["elevated", "filled", "tonal", "outlined", "text"];
const sizes = ["extra-small", "small", "medium", "large", "extra-large"];
const variantLabels = ["Elevated", "Filled", "Tonal", "Outlined", "Text"];
function buttons(iconOnly) {
  const xs = [24, 160, 296, 432, 568];
  const tag = iconOnly ? "m3e-icon-button" : "m3e-button";
  for (let i = 0; i < variants.length; i++) {
    const variant = iconOnly && i === 4 ? "standard" : variants[i];
    const attr = `variant="${variant}"${iconOnly ? ' aria-label="Favorite"' : ""}`;
    place(xs[i], 24, `<${tag} ${attr}>${iconOnly ? icon("favorite") : variantLabels[i]}</${tag}>`);
    const width = iconOnly ? (i === 1 ? ' width="narrow"' : i === 2 ? ' width="wide"' : "") : "";
    const content = iconOnly ? icon("favorite") + icon("check", "selected") : icon("add", "icon") + "Toggle";
    place(xs[i], 80, `<${tag} ${attr}${width} shape="square" toggle ${i % 2 === 0 ? "selected" : ""}>${content}</${tag}>`);
    place(xs[i], 136, `<${tag} ${attr} disabled>${iconOnly ? icon("favorite") : "Disabled"}</${tag}>`);
  }
  for (let i = 0; i < sizes.length; i++) {
    const content = iconOnly ? icon("add") : "Go" + (i === 2 ? icon("chevron_right", "trailing-icon") : "");
    place([24, 118, 228, 353, 520][i], 208, `<${tag} variant="tonal" size="${sizes[i]}" aria-label="Go">${content}</${tag}>`);
  }
}

function groups() {
  const single = ["Day", "Week", "Month"];
  const multi = ["Bold", "Italic", "Underline"];
  const group = (size, variant, names, selected, disabled = -1) =>
    `<m3e-button-group variant="connected" size="${size}">${names.map((name, i) =>
      `<m3e-button toggle variant="${variant}" size="${size}" ${selected === i ? "selected" : ""} ${disabled === i ? "disabled" : ""}>${name}</m3e-button>`).join("")}</m3e-button-group>`;
  place(24, 24, group("small", "tonal", single, 1));
  place(24, 80, group("small", "filled", multi, 0, 2));
  place(24, 150, group("medium", "outlined", single, 1));
  place(24, 234, group("large", "tonal", single, 1));
}

function connectedIcons() {
  for (const [y, size, variant] of [[24, "small", "tonal"], [94, "medium", "filled"]]) {
    const buttons = ["home", "search", "favorite"].map((name, i) =>
      `<m3e-icon-button variant="${variant}" size="${size}" toggle ${i === 1 ? "selected" : ""}
        ${y === 94 ? 'width="wide" shape="square"' : ""} aria-label="${name}">${icon(name)}</m3e-icon-button>`).join("");
    place(24, y, `<m3e-button-group variant="connected" size="${size}">${buttons}</m3e-button-group>`);
  }
}

function tabs() {
  for (const [i, y] of [[0, 24], [1, 112], [2, 188], [3, 264]]) {
    const list = ["Home", "Search", "Saved"].map((name, j) =>
      `<m3e-tab ${j === (i % 2) ? "selected" : ""} ${j === 2 ? "disabled" : ""}>${icon(["home", "search", "favorite"][j], "icon")}${name}</m3e-tab>`).join("");
    place(24, y, `<m3e-tabs variant="${i === 0 || i === 3 ? "primary" : "secondary"}" ${i >= 2 ? 'header-position="after"' : ""}
      ${i === 3 ? 'stretch style="width:560px"' : ""}>${list}</m3e-tabs>`);
  }
}

function menu() {
  const variant = sample === "menu-vibrant" ? "vibrant" : "standard";
  const width = "--m3e-menu-container-min-width:168px;--m3e-menu-container-max-width:168px";
  place(24, 24, `<m3e-button variant="filled"><m3e-menu-trigger for="actions">Actions</m3e-menu-trigger></m3e-button>`);
  main.insertAdjacentHTML("beforeend", `<m3e-menu id="actions" variant="${variant}" style="${width}">
    <m3e-menu-item>${icon("edit", "icon")}Edit</m3e-menu-item>
    <m3e-menu-item>${icon("more_vert", "icon")}<m3e-menu-trigger for="sub">More</m3e-menu-trigger></m3e-menu-item>
    <m3e-divider></m3e-divider>
    <m3e-menu-item-checkbox checked>Favorite</m3e-menu-item-checkbox>
    <m3e-menu-item disabled>${icon("delete", "icon")}Delete</m3e-menu-item>
  </m3e-menu>
  <m3e-menu id="sub" variant="${variant}" style="${width}">
    <m3e-menu-item>${icon("settings", "icon")}Settings</m3e-menu-item>
    <m3e-menu-item-radio>Pinned</m3e-menu-item-radio>
  </m3e-menu>`);
}

function fields() {
  place(24, 32, '<m3e-form-field variant="filled" hide-subscript="auto"><label slot="label" for="name">Name</label><input id="name"></m3e-form-field>');
  place(336, 32, '<m3e-form-field variant="outlined" hide-subscript="auto"><label slot="label" for="outlined">Name</label><input id="outlined" value="Ada"></m3e-form-field>');
  place(24, 120, `<m3e-form-field variant="outlined"><label slot="label" for="amount">Amount</label>
      ${icon("edit", "prefix")}${icon("info", "suffix")}<span slot="prefix-text">$</span><span slot="suffix-text">USD</span>
      <input id="amount" value="1.50" /><span slot="hint">Enter an amount</span></m3e-form-field>`);
  place(336, 120, '<m3e-form-field variant="filled" id="error"><label slot="label" for="contact">Contact</label><input id="contact" required><span slot="error">Contact is required</span></m3e-form-field>');
  place(24, 228, '<m3e-form-field variant="outlined" hide-subscript="auto"><label slot="label" for="status">Status</label><input id="status" value="Read only" disabled></m3e-form-field>');
  place(336, 228, '<m3e-form-field variant="filled" hide-subscript="auto"><label slot="label" for="notes">Notes</label><textarea id="notes" style="height:68px">First line\nSecond line</textarea></m3e-form-field>');
}

function checkboxes() {
  for (let i = 0; i < 9; i++)
    place(24 + 64 * (i % 3), 24 + 56 * Math.floor(i / 3),
      `<m3e-checkbox ${i % 3 === 1 || i === 8 ? "checked" : ""}
        ${i % 3 === 2 && i !== 8 ? "indeterminate" : ""} ${i >= 6 ? "disabled" : ""} ${i >= 3 && i < 6 ? 'data-error' : ""}></m3e-checkbox>`);
  place(24, 196, '<label style="display:flex;align-items:center;gap:8px"><m3e-checkbox checked></m3e-checkbox>Notifications</label>');
}

function switches() {
  for (let i = 0; i < 8; i++)
    place(24 + 80 * (i % 4), 24 + 62 * Math.floor(i / 4),
      `<m3e-switch ${i % 2 ? "checked" : ""} icons="${["none", "none", "both", "both", "both", "selected", "none", "selected"][i]}" ${i >= 6 ? "disabled" : ""}></m3e-switch>`);
  place(24, 152, '<label style="display:flex;align-items:center;gap:8px"><m3e-switch checked icons="selected"></m3e-switch>Bluetooth</label>');
}

function sliders() {
  const slider = (size, values, extra = "", vertical = false) =>
    `<m3e-slider size="${size}" style="${vertical ? "height" : "width"}:240px" labelled ${extra}>${values.map((v) =>
      `<m3e-slider-thumb value="${v}"></m3e-slider-thumb>`).join("")}</m3e-slider>`;
  for (let i = 0; i < sizes.length; i++)
    place(24, [24, 76, 128, 188, 266][i], slider(sizes[i], [[35, 45, 50, 55, 60][i]]));
  place(304, 24, slider("small", [40], "discrete step=10"));
  place(304, 76, slider("small", [50], "discrete step=10 disabled"));
  place(304, 128, slider("small", [30, 75], "discrete step=10"));
  place(598, 24, slider("small", [40], "orientation=vertical discrete step=10", true));
}

function dialog() {
  place(24, 24, '<m3e-button variant="filled"><m3e-dialog-trigger for="dialog">Open dialog</m3e-dialog-trigger></m3e-button>');
  main.insertAdjacentHTML("beforeend", `<m3e-dialog id="dialog" dismissible style="--m3e-dialog-min-width:320px;--m3e-dialog-max-width:320px">
    <span slot="header">Discard changes?</span>Your edits will be lost.
    <div slot="actions" end>
      <m3e-button><m3e-dialog-action>Cancel</m3e-dialog-action></m3e-button>
      <m3e-button><m3e-dialog-action>Discard</m3e-dialog-action></m3e-button>
    </div></m3e-dialog>`);
  place(172, 24, '<m3e-button variant="outlined"><m3e-dialog-trigger for="info">Info dialog</m3e-dialog-trigger></m3e-button>');
  main.insertAdjacentHTML("beforeend", `<m3e-dialog id="info" style="--m3e-dialog-min-width:320px;--m3e-dialog-max-width:320px">
    <span slot="header">${icon("info")} Appearance</span>Your preferences have been saved.
    <div slot="actions" end><m3e-button><m3e-dialog-action>Done</m3e-dialog-action></m3e-button></div>
  </m3e-dialog>`);
}

function tooltip(mode) {
  place(148, 60, `<m3e-icon-button id="settings" aria-label="Settings">${icon("settings")}</m3e-icon-button>`);
  main.insertAdjacentHTML("beforeend", mode > 0 ?
    `<m3e-rich-tooltip for="settings" position="below" style="--m3e-rich-tooltip-max-width:280px">
      <span slot="subhead">Settings</span>Change your appearance and preferences.
      ${mode === 1 ? '<div slot="actions"><m3e-button><m3e-rich-tooltip-action>Learn more</m3e-rich-tooltip-action></m3e-button></div>' : ""}
    </m3e-rich-tooltip>` : '<m3e-tooltip for="settings" position="below">Settings</m3e-tooltip>');
  if (mode === 0) {
    for (const [x, id, name, position] of [[250, "Above", "info", "above"], [352, "Left", "favorite", "before"], [454, "Right", "search", "after"]]) {
      place(x, 60, `<m3e-icon-button id="${id}" aria-label="${id}">${icon(name)}</m3e-icon-button>`);
      main.insertAdjacentHTML("beforeend", `<m3e-tooltip for="${id}" position="${position}">${id}</m3e-tooltip>`);
    }
  }
}

const fixtures = {
  button: () => buttons(false), "icon-button": () => buttons(true), connected: groups, tabs,
  menu, "menu-vibrant": menu, submenu: menu, "text-field": fields, checkbox: checkboxes, switch: switches, slider: sliders,
  dialog, tooltip: () => tooltip(0), "rich-tooltip": () => tooltip(1), "connected-icons": connectedIcons,
  "rich-hover": () => tooltip(2)
};
if (fixtures[sample])
  fixtures[sample]();
else
  drawCatalog(sample, place, icon);
await Promise.all([...document.querySelectorAll("*")].map((el) => el.updateComplete));
for (const checkbox of document.querySelectorAll("m3e-checkbox[data-error]")) {
  checkbox.setCustomValidity("Choose an option");
  checkbox.markAsTouched();
  await checkbox.updateComplete;
}
const errorField = document.querySelector("m3e-form-field#error");
if (errorField) {
  errorField.error = true;
  await errorField.updateComplete;
}
await document.fonts.ready;
window.referenceReady = true;
