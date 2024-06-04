#include <vcl.h>
#pragma hdrstop

#include <vector>

#include "MainUnit.h"
#include <injector/injector.h>

#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

using dll_injector::Injector;
using dll_injector::ProcessInfo;

static std::vector<TProcessInfo *> create_process_info_objects() {
    const std::vector<ProcessInfo> pi_values =
        dll_injector::get_process_info_values();
    std::vector<TProcessInfo *> pi_objects;
    pi_objects.reserve(pi_values.size());
    for (const ProcessInfo &pi : pi_values) {
        pi_objects.push_back(new TProcessInfo(pi));
    }
    return pi_objects;
}

static void
delete_process_info_objects(std::vector<TProcessInfo *> &pi_objects) {
    for (auto &pi : pi_objects) {
        if (pi) {
            delete pi;
            pi = nullptr;
        }
    }
}

__fastcall TMainForm::TMainForm(TComponent *Owner)
    : TForm(Owner), pi_objects_ {create_process_info_objects()} {
    update_process_name_combo_box_();
}

__fastcall TMainForm::~TMainForm() {
    delete_process_info_objects(pi_objects_);
}

void __fastcall TMainForm::set_injector(Injector *injector) {
    injector_ = injector;
    UnicodeString status_text = UnicodeString::Format(
        "Available target process architecture: x32 - %s, x64 - %s",
        (TVarRec[]) {injector_->get_load_library_x32_addr() ? "yes" : "no",
                     injector_->get_load_library_x64_addr() ? "yes" : "no"},
        1);
    status_bar->SimpleText = status_text;
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
    const TProcessInfo *pi_object =
        (TProcessInfo *) process_name_combo_box->Items->Objects[item_index];
    std::vector<std::wstring> dll_path_wstrings;
    dll_path_wstrings.reserve(dll_path_items_count);
    for (const auto &dll_path : dll_path_items) {
        dll_path_wstrings.emplace_back(dll_path.c_str());
    }
    if (!injector_->inject_multiple_dlls(pi_object->pid, dll_path_wstrings)) {
        Application->MessageBox(L"Injection failed", L"Test",
                                MB_OK | MB_ICONWARNING);
        return;
    }
}

void __fastcall TMainForm::update_process_list_event(TObject *Sender) {
    delete_process_info_objects(pi_objects_);
    pi_objects_ = create_process_info_objects();
    update_process_name_combo_box_();
}

void __fastcall TMainForm::update_process_name_combo_box_() {
    process_name_combo_box->Clear();
    for (TProcessInfo *pi : pi_objects_) {
        process_name_combo_box->AddItem(
            UnicodeString::Format("%s [%d]",
                                  (TVarRec[]) {pi->name.c_str(), pi->pid}, 1),
            pi);
    }
}
