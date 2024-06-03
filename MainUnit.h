#ifndef MainUnitH
#define MainUnitH

#include <Data.Bind.Components.hpp>
#include <Data.Bind.EngExt.hpp>
#include <System.Bindings.Outputs.hpp>
#include <System.Classes.hpp>
#include <System.ImageList.hpp>
#include <System.Rtti.hpp>
#include <Vcl.Bind.DBEngExt.hpp>
#include <Vcl.Bind.Editors.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ControlList.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.NumberBox.hpp>
#include <Vcl.Samples.Bind.Editors.hpp>
#include <Vcl.Samples.Spin.hpp>
#include <Vcl.StdCtrls.hpp>

#include <vector>

#include "injector.h"

class TProcessInfo : public TObject {
  public:
    explicit __fastcall TProcessInfo(const dll_injector::ProcessInfo &pi)
        : pid {pi.pid}, name {pi.name} {}

    uint32_t pid;
    std::wstring name;
};

class TMainForm : public TForm {
    // clang-format off
  __published:
    TListBox *dll_paths_list_box;
    // clang-format on
    TButton *remove_dll_paths_button;
    TFileOpenDialog *dlls_file_open_dialog;
    TButton *add_dll_paths_button;
    TButton *inject_button;
    TComboBox *process_name_combo_box;
    TStatusBar *status_bar;
    TGroupBox *dll_paths_group_box;
    TGroupBox *process_group_box;
    TImageList *image_list;
    void __fastcall remove_dlls_event(TObject *Sender);
    void __fastcall add_dlls_event(TObject *Sender);
    void __fastcall inject_event(TObject *Sender);
    void __fastcall update_process_list_event(TObject *Sender);

  public:
    __fastcall TMainForm(TComponent *Owner);
    __fastcall ~TMainForm();

    void __fastcall set_injector(dll_injector::Injector *injector);

  private:
    void __fastcall update_process_name_combo_box_();

    dll_injector::Injector *injector_;
    std::vector<TProcessInfo *> pi_objects_;
};

extern PACKAGE TMainForm *MainForm;

#endif
