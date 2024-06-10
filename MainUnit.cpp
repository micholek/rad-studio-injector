#include <vcl.h>
#pragma hdrstop

#include <vector>

#include "MainUnit.h"
#include <injector/injector.h>

#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

using dll_injector::Injector;
using dll_injector::Process;

__fastcall TMainForm::TMainForm(TComponent *owner)
    : TForm(owner),
      injector_((ExtractFilePath(Application->ExeName) + "/injector-helper.exe")
                    .c_str()),
      tp_objects_ {} {
    const uint64_t x64_addr = injector_.get_load_library_x64_addr();
    const uint64_t x32_addr = injector_.get_load_library_x32_addr();
    if (!x64_addr) {
        Application->MessageBox(
            L"64-bit version of LoadLibraryW has not been found.\n"
            L"Injecting into 64-bit processes will not be possible.",
            L"Warning", MB_OK | MB_ICONWARNING);
    }
    if (!x32_addr) {
        Application->MessageBox(
            L"32-bit version of LoadLibraryW has not been found.\n"
            L"Injecting into 32-bit processes will not be possible.",
            L"Warning", MB_OK | MB_ICONWARNING);
    }
    const UnicodeString status_text = UnicodeString::Format(
        "Available target process architecture: x32 - %s, x64 - %s",
        (TVarRec[]) {x32_addr ? "yes" : "no", x64_addr ? "yes" : "no"}, 1);
    status_bar->SimpleText = status_text;
    create_tprocess_objects_();
    update_process_name_combo_box_();
}

__fastcall TMainForm::~TMainForm() {
    delete_tprocess_objects_();
}

void __fastcall TMainForm::remove_dlls_event(TObject *Sender) {
    dll_paths_list_box->DeleteSelected();
}

void __fastcall TMainForm::add_dlls_event(TObject *Sender) {
    if (!dlls_file_open_dialog->Execute()) {
        return;
    }
    for (const auto &dll_path : dlls_file_open_dialog->Files) {
        if (dll_paths_list_box->Items->IndexOf(dll_path) == -1) {
            dll_paths_list_box->AddItem(dll_path, nullptr);
        }
    }
}

void __fastcall TMainForm::inject_event(TObject *Sender) {
    const int item_index = process_name_combo_box->ItemIndex;
    if (item_index == -1) {
        Application->MessageBox(L"Select a valid process", L"Not enough data",
                                MB_OK | MB_ICONINFORMATION);
        return;
    }
    TStrings *dll_path_items = dll_paths_list_box->Items;
    const int dll_path_items_count = dll_path_items->Count;
    if (!dll_path_items_count) {
        Application->MessageBox(L"Add DLLs", L"Not enough data",
                                MB_OK | MB_ICONINFORMATION);
        return;
    }
    const TProcess *tp_object =
        (TProcess *) process_name_combo_box->Items->Objects[item_index];
    std::vector<std::wstring> dll_path_wstrings;
    dll_path_wstrings.reserve(dll_path_items_count);
    for (const auto &dll_path : dll_path_items) {
        dll_path_wstrings.emplace_back(dll_path.c_str());
    }
    if (!injector_.inject_dlls(tp_object->pid, dll_path_wstrings)) {
        Application->MessageBox(L"Injection failed", L"Failure",
                                MB_OK | MB_ICONWARNING);
    }
}

void __fastcall TMainForm::update_process_list_event(TObject *Sender) {
    delete_tprocess_objects_();
    create_tprocess_objects_();
    update_process_name_combo_box_();
}

void __fastcall TMainForm::update_process_name_combo_box_() {
    process_name_combo_box->Clear();
    for (TProcess *tp : tp_objects_) {
        if (tp) {
            process_name_combo_box->AddItem(
                UnicodeString::Format(
                    "%s [%d]", (TVarRec[]) {tp->name.c_str(), tp->pid}, 1),
                tp);
        }
    }
}

void __fastcall TMainForm::create_tprocess_objects_() {
    const std::vector<Process> p_objects =
        injector_.fetch_process_objects_all();
    tp_objects_.clear();
    tp_objects_.reserve(p_objects.size());
    for (const Process &p : p_objects) {
        tp_objects_.push_back(new TProcess(p));
    }
}

void __fastcall TMainForm::delete_tprocess_objects_() {
    for (auto &tp : tp_objects_) {
        if (tp) {
            delete tp;
            tp = nullptr;
        }
    }
}
