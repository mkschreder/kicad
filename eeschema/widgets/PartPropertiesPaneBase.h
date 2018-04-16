///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 14 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __PARTPROPERTIESPANEBASE_H__
#define __PARTPROPERTIESPANEBASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/grid.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class PartPropertiesPaneBase
///////////////////////////////////////////////////////////////////////////////
class PartPropertiesPaneBase : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText1;
		wxTextCtrl* m_textPartID;
		wxStaticText* m_staticText6;
		wxTextCtrl* m_textDesignator;
		wxStaticText* m_staticText2;
		wxTextCtrl* m_textComment;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_textDatasheet;
		wxStaticText* m_staticText5;
		wxTextCtrl* m_textMPN;
		wxBitmapButton* m_bpButton1;
		wxGrid* m_gridFields;
		wxButton* m_buttonLoadFields;
		wxButton* m_buttonAddField;
		wxButton* m_buttonDeleteField;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void onPartIDKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void onPartIDChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void onPartIDTextEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void onDesignatorKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void onCommentKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void onCommentChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void onDatasheetKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void onMPNKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void onRefreshFromMPN( wxCommandEvent& event ) { event.Skip(); }
		virtual void onGridCellChanged( wxGridEvent& event ) { event.Skip(); }
		virtual void onLoadFieldsClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onAddField( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		PartPropertiesPaneBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~PartPropertiesPaneBase();
	
};

#endif //__PARTPROPERTIESPANEBASE_H__
