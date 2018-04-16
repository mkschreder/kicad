#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/font.h>
#include <wx/grid.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/panel.h>

#include <vector>

#include "../lib_field.h"
#include "../lib_edit_frame.h"
#include "../class_libentry.h"

#include "PartPropertiesPaneBase.h"

class PartPropertiesPane : public PartPropertiesPaneBase
{
    public:
		PartPropertiesPane( LIB_EDIT_FRAME *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL );
		virtual ~PartPropertiesPane();
        void Refresh();

        void SetPart( LIB_PART *part);
	protected:
		virtual void onGridCellChanged( wxGridEvent& event ) override;
		virtual void onSize( wxSizeEvent& event ) override;
		virtual void onAddField( wxCommandEvent& event ) override;

        // parameters change
		virtual void onPartIDChange( wxCommandEvent& event ) override;
    	virtual void onPartIDKillFocus( wxFocusEvent& event ) override;
		virtual void onPartIDTextEnter( wxCommandEvent& event ) override;

        virtual void onDesignatorKillFocus( wxFocusEvent& event ) override;
		virtual void onCommentKillFocus( wxFocusEvent& event ) override;
		virtual void onDatasheetKillFocus( wxFocusEvent& event ) override;
		virtual void onMPNKillFocus( wxFocusEvent& event ) override;
    private:
        std::vector<LIB_FIELD> m_fields;
        LIB_PART *m_part;
        LIB_EDIT_FRAME*    m_parent;

        void AutosizeFieldsGrid();
        void RefreshGrid();
        void NotifyParent();
        void UpdateFieldValue(LIB_FIELD *f, const wxString &value);
        void UpdateFieldValue(const wxString &name, const wxString &value);
};

