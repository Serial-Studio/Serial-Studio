// Serial Studio chat viewer HTML sanitizer.
//
// Tight allow-list sanitizer for the AI chat transcript. Provider responses
// flow through Showdown -> this function -> innerHTML, so anything not in the
// allow-list must be stripped before the assignment.
//
// Threat model: a poisoned model, a prompt-injected response, or a
// malicious system prompt loaded from a third-party project file emits HTML
// payloads designed to call QWebChannel methods (window.ai.approve, etc.) or
// exfiltrate state via JS execution. We assume Showdown emits HTML and the
// adversary controls the input string.
//
// Design choices:
// - Allow-list of tags and attributes. Anything else is dropped.
// - href / src values are URL-validated: only http(s), mailto, qrc, relative.
// - Inline event handlers (on*) are universally rejected.
// - <script>, <style>, <iframe>, <object>, <embed>, <link>, <meta>, <base>,
//   <form>, <input>, <button>, <textarea>, <select>, <noscript>, <svg>,
//   <math>, <details>/<summary> are dropped wholesale (tag plus subtree).
// - <a target> is forced to _blank and rel is forced to noopener noreferrer
//   (defense in depth even though Showdown's openLinksInNewWindow is false).
// - data: URLs are rejected except for known-safe image types (none enabled).
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
(function (global) {
  "use strict";

  // Allowed element names. Everything else is replaced by its text content.
  var ALLOWED_TAGS = {
    a: 1, abbr: 1, b: 1, blockquote: 1, br: 1, caption: 1, code: 1,
    del: 1, div: 1, em: 1, h1: 1, h2: 1, h3: 1, h4: 1, h5: 1, h6: 1,
    hr: 1, i: 1, img: 1, ins: 1, kbd: 1, li: 1, ol: 1, p: 1, pre: 1,
    q: 1, s: 1, samp: 1, small: 1, span: 1, strong: 1, sub: 1, sup: 1,
    table: 1, tbody: 1, td: 1, tfoot: 1, th: 1, thead: 1, tr: 1, u: 1,
    ul: 1, var: 1
  };

  // Tags dropped along with their entire subtree. Defense in depth: even if
  // a future allow-list mistake lets one of these through, it never executes
  // because we strip the node before serializing.
  var DROP_SUBTREE_TAGS = {
    script: 1, style: 1, iframe: 1, frame: 1, frameset: 1, object: 1,
    embed: 1, link: 1, meta: 1, base: 1, form: 1, input: 1, button: 1,
    textarea: 1, select: 1, option: 1, noscript: 1, svg: 1, math: 1,
    template: 1, details: 1, summary: 1, dialog: 1, applet: 1
  };

  // Per-tag attribute allow-list. Anything not listed is dropped. Class and
  // data-* are global but enumerated below for clarity.
  var ALLOWED_ATTRS = {
    "*":  { "class": 1, "title": 1, "lang": 1, "dir": 1, "id": 1 },
    a:    { href: 1, name: 1 },
    img:  { src: 1, alt: 1, width: 1, height: 1 },
    td:   { colspan: 1, rowspan: 1, align: 1 },
    th:   { colspan: 1, rowspan: 1, align: 1, scope: 1 },
    ol:   { start: 1, type: 1, reversed: 1 },
    li:   { value: 1 },
    code: { "data-lang": 1 },
    pre:  { "data-copy": 1 }
  };

  // Schemes accepted in href / src. Anything else is removed.
  var SAFE_URL_SCHEMES = ["http:", "https:", "mailto:", "qrc:"];

  function isSafeUrl(value) {
    if (typeof value !== "string") return false;
    var trimmed = value.trim();
    if (trimmed === "") return false;
    // Relative URLs (no scheme) are fine.
    if (!/^[a-z][a-z0-9+.-]*:/i.test(trimmed)) return true;
    var lower = trimmed.toLowerCase();
    for (var i = 0; i < SAFE_URL_SCHEMES.length; i++) {
      if (lower.indexOf(SAFE_URL_SCHEMES[i]) === 0) return true;
    }
    return false;
  }

  function attrAllowed(tagName, attrName) {
    if (/^on/i.test(attrName)) return false;
    if (attrName === "style") return false;
    if (attrName.indexOf("data-") === 0) {
      // Allow any data-* attribute; values are escaped on the way out.
      return true;
    }
    var globalAttrs = ALLOWED_ATTRS["*"];
    if (globalAttrs && globalAttrs[attrName]) return true;
    var perTag = ALLOWED_ATTRS[tagName];
    return !!(perTag && perTag[attrName]);
  }

  function sanitizeAttributes(el) {
    var tagName = el.tagName.toLowerCase();
    // Snapshot the attribute list because we mutate while iterating.
    var attrs = [];
    for (var i = 0; i < el.attributes.length; i++) {
      var a = el.attributes[i];
      attrs.push({ name: a.name, value: a.value });
    }
    for (var j = 0; j < attrs.length; j++) {
      var name = attrs[j].name.toLowerCase();
      var value = attrs[j].value;
      if (!attrAllowed(tagName, name)) {
        el.removeAttribute(attrs[j].name);
        continue;
      }
      if ((name === "href" || name === "src") && !isSafeUrl(value)) {
        el.removeAttribute(attrs[j].name);
        continue;
      }
    }
    // Force safe anchor behaviour. innerHTML strings can carry target=_self
    // pointing at a frame name; this resets it.
    if (tagName === "a" && el.hasAttribute("href")) {
      el.setAttribute("target", "_blank");
      el.setAttribute("rel", "noopener noreferrer");
    }
  }

  function walk(node) {
    // Iterate over a snapshot because we may remove or replace children.
    var children = [];
    for (var i = 0; i < node.childNodes.length; i++) {
      children.push(node.childNodes[i]);
    }
    for (var k = 0; k < children.length; k++) {
      var child = children[k];
      if (child.nodeType === Node.ELEMENT_NODE) {
        var tagName = child.tagName.toLowerCase();
        if (DROP_SUBTREE_TAGS[tagName]) {
          // Remove element and subtree entirely.
          node.removeChild(child);
          continue;
        }
        if (!ALLOWED_TAGS[tagName]) {
          // Replace disallowed element with its text content so the surrounding
          // structure stays readable but the disallowed markup is gone.
          var text = document.createTextNode(child.textContent || "");
          node.replaceChild(text, child);
          continue;
        }
        sanitizeAttributes(child);
        walk(child);
      } else if (child.nodeType === Node.COMMENT_NODE) {
        // HTML comments can carry conditional IE-style instructions. Drop them.
        node.removeChild(child);
      }
      // Text, CDATA, etc. left alone.
    }
  }

  function sanitize(html) {
    if (typeof html !== "string" || html === "") return "";
    var parser = new DOMParser();
    var doc;
    try {
      doc = parser.parseFromString("<div id=\"__ss_root\">" + html + "</div>",
                                   "text/html");
    } catch (e) {
      return "";
    }
    var root = doc.getElementById("__ss_root");
    if (!root) return "";
    walk(root);
    return root.innerHTML;
  }

  global.SerialStudioSanitize = { sanitize: sanitize };
})(window);
