#include "PartPropertiesPane.h"
#include "../template_fieldnames.h"

PartPropertiesPane::PartPropertiesPane( LIB_EDIT_FRAME *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    PartPropertiesPaneBase( parent, id, pos, size, style ),
    m_parent(parent)
{

}

PartPropertiesPane::~PartPropertiesPane()
{

}

void PartPropertiesPane::onGridCellChanged( wxGridEvent& event ) {
    int row = event.GetRow();
    int col = event.GetCol();

    //wxAssert(row <= (int)m_fields.size());

    if(col == 0){
        m_fields[row].SetName(m_gridFields->GetCellValue(row, col));
    } else if(col == 1){
        m_fields[row].SetText(m_gridFields->GetCellValue(row, col));
    } else {
        return;
    }

    m_part->SetFields(m_fields);

    NotifyParent();
}

void PartPropertiesPane::NotifyParent(){
    m_parent->OnModify();

    m_parent->UpdateAliasSelectList();
    m_parent->UpdatePartSelectList();
    m_parent->DisplayLibInfos();
    m_parent->Refresh();
}

void PartPropertiesPane::onSize( wxSizeEvent& event ) {
    //m_gridFields->AutoSize();
    AutosizeFieldsGrid();
}

void PartPropertiesPane::onAddField( wxCommandEvent& event ) {
    m_fields.push_back(LIB_FIELD(m_fields.size()));
    m_gridFields->InsertRows(m_gridFields->GetNumberRows(), 1);
}

void PartPropertiesPane::onPartIDChange( wxCommandEvent& event ) {
}

void PartPropertiesPane::onPartIDKillFocus( wxFocusEvent& event ) {
    wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue(m_part->GetField( VALUE ), m_textPartID->GetValue());
}

void PartPropertiesPane::onPartIDTextEnter( wxCommandEvent& event ) {
    wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue(m_part->GetField( VALUE ), m_textPartID->GetValue());
}

void PartPropertiesPane::onDesignatorKillFocus( wxFocusEvent& event ) {
     wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue(m_part->GetField( REFERENCE ), m_textDesignator->GetValue());
}

void PartPropertiesPane::onCommentKillFocus( wxFocusEvent& event ) {
    wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue("Comment", m_textComment->GetValue());
}

void PartPropertiesPane::onDatasheetKillFocus( wxFocusEvent& event ) {
    wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue(m_part->GetField( DATASHEET ), m_textDatasheet->GetValue());
}

void PartPropertiesPane::onMPNKillFocus( wxFocusEvent& event ) {
    wxASSERT(m_part);
    if(!m_part) return;

    UpdateFieldValue("MPN", m_textMPN->GetValue());
}

void PartPropertiesPane::UpdateFieldValue(LIB_FIELD *f, const wxString &value){
    wxASSERT(f);

    if(!f) return;

    if(f->GetId() < MANDATORY_FIELDS){
        m_parent->OnUpdateFieldValue(f, value);
    } else {
        f->SetText(value);
    }

    RefreshGrid();

    NotifyParent();
}

void PartPropertiesPane::UpdateFieldValue(const wxString &name, const wxString &value){
    printf("update field %ls\n", name.wc_str());
    wxASSERT(m_parent != nullptr);
    if(!m_parent) return;

    LIB_FIELD *f = m_part->FindField(name);
    if(!f) {
        LIB_FIELD nf = LIB_FIELD(m_fields.size());
        nf.SetName(name);
        nf.SetText(value);
        m_fields.push_back(nf);
        m_part->SetFields(m_fields);
    } else {
        m_parent->OnUpdateFieldValue(f, value);
    }

    RefreshGrid();

    NotifyParent();
}

void PartPropertiesPane::SetPart( LIB_PART *part){
    m_part = part;

    Refresh();
}

void PartPropertiesPane::Refresh(){
    if(!m_part) {
        return;
    }

    // set core fields
    m_textPartID->SetValue(m_part->GetField( VALUE )->GetText());
    m_textDesignator->SetValue(m_part->GetField( REFERENCE )->GetText());
    m_textDatasheet->SetValue(m_part->GetField( DATASHEET )->GetText());
    LIB_FIELD *f = nullptr;
    if(!!(f = m_part->FindField( "Comment" ))) m_textComment->SetValue(f->GetText());
    else m_textComment->SetValue("");
    if(!!(f = m_part->FindField( "MPN" ))) m_textMPN->SetValue(f->GetText());
    else m_textMPN->SetValue("");

    RefreshGrid();
}

void PartPropertiesPane::RefreshGrid(){
    m_fields.clear();
    m_part->GetFields(m_fields);
    //m_partProperties->Clear();

    // load part fields into the grid
    if(m_gridFields->GetNumberRows() > 0) {
        m_gridFields->DeleteRows(0, m_gridFields->GetNumberRows());
    }

    m_gridFields->InsertRows(0, m_fields.size());

    //m_partProperties->Append(new wxPropertyCategory(_("Properties")));

    size_t idx = 0;
    for(auto it = m_fields.begin(); it != m_fields.end(); ++it, idx++){
        auto &f = *it;
        m_gridFields->SetCellValue(idx, 0, f.GetName());
        m_gridFields->SetCellValue(idx, 1, f.GetText());
        //m_partProperties->Append(new wxStringProperty(f.GetName(), f.GetName(), f.GetText()));
    }

    AutosizeFieldsGrid();
    //m_gridFields->AutoSize();

}

void PartPropertiesPane::AutosizeFieldsGrid(){
    long width = m_gridFields->GetRowLabelSize();
    long num_cols = m_gridFields->GetNumberCols();
    for(int col = 0; col < num_cols - 1; col++){
        m_gridFields->AutoSizeColumn(col);
        width += m_gridFields->GetColSize(col);
    }

    int size = m_gridFields->GetClientSize().GetWidth() - width - 1;
    if(size < 80) size = 80;

    m_gridFields->SetColSize(num_cols - 1, size);
}
