/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/Utilities.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

//--------------------------------------------------------------------------------------------------
// Native NSAlert bridge
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a Qt message-box icon to its NSAlert equivalent.
 */
static NSAlertStyle alertStyleFor(QMessageBox::Icon icon)
{
  switch (icon) {
    case QMessageBox::Critical: return NSAlertStyleCritical;
    case QMessageBox::Warning:  return NSAlertStyleWarning;
    default:                    return NSAlertStyleInformational;
  }
}

/**
 * @brief Translates a Qt standard-button set to up to three NSAlert buttons.
 *
 * NSAlert only accepts three buttons (primary, secondary, tertiary); Qt's
 * @c QMessageBox can carry many more. We map the common subset the app
 * actually uses and fall back to @c "OK" when no button flags are given.
 */
static NSArray<NSString *> *buttonTitlesFor(QMessageBox::StandardButtons bt,
                                            const ButtonTextMap& overrides)
{
  // Preserve Qt's render order so button sequence matches other platforms
  static const QMessageBox::StandardButton kOrder[] = {
    QMessageBox::Ok, QMessageBox::Save, QMessageBox::SaveAll,
    QMessageBox::Yes, QMessageBox::YesToAll, QMessageBox::Retry,
    QMessageBox::Apply, QMessageBox::Open, QMessageBox::Ignore,
    QMessageBox::No, QMessageBox::NoToAll, QMessageBox::Abort,
    QMessageBox::Discard, QMessageBox::Close, QMessageBox::Cancel,
    QMessageBox::Reset, QMessageBox::RestoreDefaults, QMessageBox::Help
  };

  auto translate = [&overrides](QMessageBox::StandardButton b) -> NSString * {
    if (overrides.contains(b))
      return overrides.value(b).toNSString();

    switch (b) {
      case QMessageBox::Ok:               return @"OK";
      case QMessageBox::Save:             return @"Save";
      case QMessageBox::SaveAll:          return @"Save All";
      case QMessageBox::Open:             return @"Open";
      case QMessageBox::Yes:              return @"Yes";
      case QMessageBox::YesToAll:         return @"Yes to All";
      case QMessageBox::No:               return @"No";
      case QMessageBox::NoToAll:          return @"No to All";
      case QMessageBox::Abort:            return @"Abort";
      case QMessageBox::Retry:            return @"Retry";
      case QMessageBox::Ignore:           return @"Ignore";
      case QMessageBox::Close:            return @"Close";
      case QMessageBox::Cancel:           return @"Cancel";
      case QMessageBox::Discard:          return @"Discard";
      case QMessageBox::Help:             return @"Help";
      case QMessageBox::Apply:            return @"Apply";
      case QMessageBox::Reset:            return @"Reset";
      case QMessageBox::RestoreDefaults:  return @"Restore Defaults";
      default:                            return @"OK";
    }
  };

  NSMutableArray<NSString *> *titles = [NSMutableArray array];
  for (QMessageBox::StandardButton b : kOrder) {
    if (bt & b)
      [titles addObject:translate(b)];
  }
  if (titles.count == 0)
    [titles addObject:@"OK"];

  return titles;
}

/**
 * @brief Matches the NSAlert return code back to a Qt StandardButton.
 *
 * NSAlert returns 1000, 1001, 1002 for first/second/third button. We map
 * those indices into the same @c kOrder list used to build the title list
 * so the caller sees the Qt value corresponding to the clicked button.
 */
static QMessageBox::StandardButton mapNSAlertReturn(
  NSModalResponse response, QMessageBox::StandardButtons bt)
{
  static const QMessageBox::StandardButton kOrder[] = {
    QMessageBox::Ok, QMessageBox::Save, QMessageBox::SaveAll,
    QMessageBox::Yes, QMessageBox::YesToAll, QMessageBox::Retry,
    QMessageBox::Apply, QMessageBox::Open, QMessageBox::Ignore,
    QMessageBox::No, QMessageBox::NoToAll, QMessageBox::Abort,
    QMessageBox::Discard, QMessageBox::Close, QMessageBox::Cancel,
    QMessageBox::Reset, QMessageBox::RestoreDefaults, QMessageBox::Help
  };

  const int index = static_cast<int>(response - NSAlertFirstButtonReturn);
  int matched = 0;
  for (QMessageBox::StandardButton b : kOrder) {
    if (bt & b) {
      if (matched == index)
        return b;

      ++matched;
    }
  }
  return QMessageBox::Ok;
}

/**
 * @brief Native NSAlert replacement for @c showMessageBox on macOS.
 *
 * Called from @c Misc::Utilities::showMessageBox via a weak symbol so the
 * same Utilities.cpp compiles unchanged on other platforms.
 */
int Misc_Utilities_showNativeMessageBox(
  const QString& text, const QString& informativeText, QMessageBox::Icon icon,
  const QString& windowTitle, QMessageBox::StandardButtons bt,
  QMessageBox::StandardButton defaultButton,
  const ButtonTextMap& buttonTexts)
{
  // NSAlert does not render a window title by convention
  Q_UNUSED(windowTitle);

  NSAlert *alert = [[NSAlert alloc] init];
  alert.alertStyle = alertStyleFor(icon);

  // Headline text + subordinate informative text, same split as QMessageBox
  alert.messageText     = text.toNSString();
  alert.informativeText = informativeText.toNSString();

  // Populate up to three buttons in the order Qt expects them
  NSArray<NSString *> *titles = buttonTitlesFor(bt, buttonTexts);
  for (NSUInteger i = 0; i < titles.count && i < 3; ++i)
    [alert addButtonWithTitle:titles[i]];

  // Highlight the requested default button (first is highlighted otherwise)
  if (defaultButton != QMessageBox::NoButton) {
    int idx = 0;
    for (NSString *title in titles) {
      (void)title;
      if (mapNSAlertReturn(NSAlertFirstButtonReturn + idx, bt) == defaultButton) {
        if (idx < static_cast<int>(alert.buttons.count))
          alert.window.defaultButtonCell = [alert.buttons[idx] cell];

        break;
      }
      ++idx;
    }
  }

  const NSModalResponse response = [alert runModal];
  return static_cast<int>(mapNSAlertReturn(response, bt));
}
