///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 14 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "PartPropertiesPaneBase.h"

///////////////////////////////////////////////////////////////////////////

PartPropertiesPaneBase::PartPropertiesPaneBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Properties") ), wxVERTICAL );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_staticText1 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Part ID"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	gSizer1->Add( m_staticText1, 0, wxALL, 5 );
	
	m_textPartID = new wxTextCtrl( sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textPartID->SetToolTip( wxT("Manufacturer part number that identifies this compoent") );
	
	gSizer1->Add( m_textPartID, 0, wxEXPAND, 5 );
	
	m_staticText6 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Designator"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	gSizer1->Add( m_staticText6, 0, wxALL, 5 );
	
	m_textDesignator = new wxTextCtrl( sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_textDesignator, 0, wxEXPAND, 5 );
	
	m_staticText2 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	gSizer1->Add( m_staticText2, 0, wxALL, 5 );
	
	m_textComment = new wxTextCtrl( sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_textComment, 0, wxEXPAND, 5 );
	
	m_staticText4 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Datasheet"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	gSizer1->Add( m_staticText4, 0, wxALL, 5 );
	
	m_textDatasheet = new wxTextCtrl( sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_textDatasheet, 0, wxEXPAND, 5 );
	
	m_staticText5 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Manufacturer Part Number"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	gSizer1->Add( m_staticText5, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_textMPN = new wxTextCtrl( sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_textMPN, 1, wxEXPAND, 5 );
	
	m_bpButton1 = new wxBitmapButton( sbSizer2->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bSizer5->Add( m_bpButton1, 0, 0, 5 );
	
	
	gSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
	
	
	sbSizer2->Add( gSizer1, 1, wxALL|wxEXPAND, 5 );
	
	
	bMainSizer->Add( sbSizer2, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Parameters") ), wxVERTICAL );
	
	m_gridFields = new wxGrid( sbSizer1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	
	// Grid
	m_gridFields->CreateGrid( 0, 2 );
	m_gridFields->EnableEditing( true );
	m_gridFields->EnableGridLines( true );
	m_gridFields->EnableDragGridSize( false );
	m_gridFields->SetMargins( 0, 0 );
	
	// Columns
	m_gridFields->EnableDragColMove( false );
	m_gridFields->EnableDragColSize( true );
	m_gridFields->SetColLabelSize( 30 );
	m_gridFields->SetColLabelValue( 0, wxT("Field") );
	m_gridFields->SetColLabelValue( 1, wxT("Value") );
	m_gridFields->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_gridFields->EnableDragRowSize( true );
	m_gridFields->SetRowLabelSize( 30 );
	m_gridFields->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_gridFields->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	m_gridFields->SetMinSize( wxSize( -1,200 ) );
	
	sbSizer1->Add( m_gridFields, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	m_buttonLoadFields = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Load From Octopart"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( m_buttonLoadFields, 0, wxALL, 5 );
	
	
	bSizer31->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_buttonAddField = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( m_buttonAddField, 0, wxALL, 5 );
	
	m_buttonDeleteField = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( m_buttonDeleteField, 0, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer31, 0, wxEXPAND, 5 );
	
	
	bMainSizer->Add( sbSizer1, 1, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Footprint") ), wxVERTICAL );
	
	
	bMainSizer->Add( sbSizer3, 1, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Models") ), wxVERTICAL );
	
	
	bMainSizer->Add( sbSizer4, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bMainSizer );
	this->Layout();
	bMainSizer->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( PartPropertiesPaneBase::onSize ) );
	m_textPartID->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onPartIDKillFocus ), NULL, this );
	m_textPartID->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PartPropertiesPaneBase::onPartIDChange ), NULL, this );
	m_textPartID->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( PartPropertiesPaneBase::onPartIDTextEnter ), NULL, this );
	m_textDesignator->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onDesignatorKillFocus ), NULL, this );
	m_textComment->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onCommentKillFocus ), NULL, this );
	m_textComment->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PartPropertiesPaneBase::onCommentChange ), NULL, this );
	m_textDatasheet->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onDatasheetKillFocus ), NULL, this );
	m_textMPN->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onMPNKillFocus ), NULL, this );
	m_bpButton1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onRefreshFromMPN ), NULL, this );
	m_gridFields->Connect( wxEVT_GRID_CELL_CHANGED, wxGridEventHandler( PartPropertiesPaneBase::onGridCellChanged ), NULL, this );
	m_buttonLoadFields->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onLoadFieldsClick ), NULL, this );
	m_buttonAddField->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onAddField ), NULL, this );
}

PartPropertiesPaneBase::~PartPropertiesPaneBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( PartPropertiesPaneBase::onSize ) );
	m_textPartID->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onPartIDKillFocus ), NULL, this );
	m_textPartID->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PartPropertiesPaneBase::onPartIDChange ), NULL, this );
	m_textPartID->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( PartPropertiesPaneBase::onPartIDTextEnter ), NULL, this );
	m_textDesignator->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onDesignatorKillFocus ), NULL, this );
	m_textComment->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onCommentKillFocus ), NULL, this );
	m_textComment->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PartPropertiesPaneBase::onCommentChange ), NULL, this );
	m_textDatasheet->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onDatasheetKillFocus ), NULL, this );
	m_textMPN->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( PartPropertiesPaneBase::onMPNKillFocus ), NULL, this );
	m_bpButton1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onRefreshFromMPN ), NULL, this );
	m_gridFields->Disconnect( wxEVT_GRID_CELL_CHANGED, wxGridEventHandler( PartPropertiesPaneBase::onGridCellChanged ), NULL, this );
	m_buttonLoadFields->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onLoadFieldsClick ), NULL, this );
	m_buttonAddField->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PartPropertiesPaneBase::onAddField ), NULL, this );
	
}
