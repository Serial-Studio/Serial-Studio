/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/docview.h>
#include "mdfdocument.h"

namespace mdf::viewer {
 class MdfView : public wxView  {
  public:
   MdfView() = default;
   MdfDocument* GetDocument() const;

   bool OnCreate(wxDocument* doc, long flags) override;
   bool OnClose(bool del) override;

   void OnDraw(wxDC *dc) override;
   void OnUpdate(wxView *sender, wxObject *hint = nullptr) override;

  private:

   wxDECLARE_DYNAMIC_CLASS(MdfView);

 };
}





