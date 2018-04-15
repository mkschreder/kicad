/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <wx/tooltip.h>
#include <wx/hyperlink.h>
#include <wx/url.h>

#include <fctsys.h>
#include <pgm_base.h>
#include <kiway.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <base_units.h>

#include <sch_reference_list.h>
#include <class_library.h>
#include <sch_component.h>
#include <dialog_helpers.h>
#include <sch_validators.h>
#include <kicad_device_context.h>
#include <symbol_lib_table.h>

#include <bitmaps.h>

#include <dialog_edit_component_in_schematic_fbp.h>
#include <invoke_sch_dialog.h>

#ifdef KICAD_SPICE
#include <dialog_spice_model.h>
#include <netlist_exporter_pspice.h>
#endif /* KICAD_SPICE */

#include "common.h"
#include "eda_doc.h"
#include <list>


/**
 * Dialog used to edit #SCH_COMPONENT objects in a schematic.
 *
 * This is derived from DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP which is maintained by
 * wxFormBuilder.  Do not auto-generate this class or file, it is hand coded.
 */
class DIALOG_EDIT_COMPONENT_IN_SCHEMATIC : public DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP
{
public:
    DIALOG_EDIT_COMPONENT_IN_SCHEMATIC( SCH_EDIT_FRAME* aParent );

    /**
     * Initialize controls with \a aComponent.
     *.
     * @param aComponent The component to edit.
     */
    void InitBuffers( SCH_COMPONENT* aComponent );

    SCH_EDIT_FRAME* GetParent() { return dynamic_cast< SCH_EDIT_FRAME* >( wxDialog::GetParent() ); }

private:

    friend class SCH_EDIT_FRAME;

    SCH_COMPONENT*  m_cmp;
    LIB_PART*       m_part;
    bool            m_skipCopyFromPanel;

    static int      s_SelectedRow;

    /// a copy of the edited symbol's SCH_FIELDs
    SCH_FIELDS      m_FieldsBuf;

    void setSelectedFieldNdx( int aFieldNdx );

    int getSelectedFieldNdx();

    /**
     * Sets the values displayed on the panel according to the currently selected field row.
     */
    void copySelectedFieldToPanel();

    /**
     * Copy the values displayed on the panel fields to the currently selected field.
     *
     * @return bool - true if all fields are OK, else false if the user has put
     *   bad data into a field, and this value can be used to deny a row change.
     */
    bool copyPanelToSelectedField();

    void copyOptionsToPanel();

    void copyPanelToOptions();

    void setRowItem( int aFieldNdx, const wxString& aName, const wxString& aValue );

    void setRowItem( int aFieldNdx, const SCH_FIELD& aField )
    {
        // Use default field name for mandatory fields, because they are transalted
        // according to the current locale
        wxString f_name;

        if( aFieldNdx < MANDATORY_FIELDS )
            f_name = TEMPLATE_FIELDNAME::GetDefaultFieldName( aFieldNdx );
        else
            f_name = aField.GetName( false );

        setRowItem( aFieldNdx, f_name, aField.GetText() );
    }

    // event handlers
    void OnCloseDialog( wxCloseEvent& event ) override;
    void OnListItemDeselected( wxListEvent& event ) override;
    void OnListItemSelected( wxListEvent& event ) override;
    void OnCancelButtonClick( wxCommandEvent& event ) override;
    void OnOKButtonClick( wxCommandEvent& event ) override;
    void SetInitCmp( wxCommandEvent& event ) override;
    void UpdateFields( wxCommandEvent& event ) override;
    void addFieldButtonHandler( wxCommandEvent& event ) override;
    void deleteFieldButtonHandler( wxCommandEvent& event ) override;
    void moveUpButtonHandler( wxCommandEvent& event ) override;
    void moveDownButtonHandler( wxCommandEvent& event ) override;
    void showButtonHandler( wxCommandEvent& event ) override;
    void OnTestChipName( wxCommandEvent& event ) override;
    void OnSelectChipName( wxCommandEvent& event ) override;
    void OnInitDlg( wxInitDialogEvent& event ) override
    {
        TransferDataToWindow();

        // Now all widgets have the size fixed, call FinishDialogSettings
        FinishDialogSettings();
    }

    void EditSpiceModel( wxCommandEvent& event ) override;

    SCH_FIELD* findField( const wxString& aFieldName );

    /**
     * Update the listbox showing fields according to the field's text.
     *
     * This must be called after a text change in fields if this change is not an edition.
     */
    void updateDisplay()
    {
        fieldListCtrl->DeleteAllItems();

        for( unsigned ii = 0; ii < m_FieldsBuf.size(); ii++ )
            setRowItem( ii, m_FieldsBuf[ii] );
    }
};


int DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::s_SelectedRow;


void SCH_EDIT_FRAME::EditComponent( SCH_COMPONENT* aComponent )
{
    wxCHECK_RET( aComponent != NULL && aComponent->Type() == SCH_COMPONENT_T,
                 wxT( "Invalid component object pointer.  Bad Programmer!" ) );

    m_canvas->SetIgnoreMouseEvents( true );

    DIALOG_EDIT_COMPONENT_IN_SCHEMATIC* dlg = new DIALOG_EDIT_COMPONENT_IN_SCHEMATIC( this );

    dlg->InitBuffers( aComponent );

    // make sure the chipnameTextCtrl is wide enough to hold any unusually long chip names:
    EnsureTextCtrlWidth( dlg->chipnameTextCtrl );

    // This dialog itself subsequently can invoke a KIWAY_PLAYER as a quasimodal
    // frame. Therefore this dialog as a modal frame parent, MUST be run under
    // quasimodal mode for the quasimodal frame support to work.  So don't use
    // the QUASIMODAL macros here.
    int ret = dlg->ShowQuasiModal();

    if( m_autoplaceFields )
        aComponent->AutoAutoplaceFields( GetScreen() );

    m_canvas->SetIgnoreMouseEvents( false );
    m_canvas->MoveCursorToCrossHair();
    dlg->Destroy();

    if( ret == wxID_OK )
        GetCanvas()->Refresh();
}


DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::DIALOG_EDIT_COMPONENT_IN_SCHEMATIC( SCH_EDIT_FRAME* aParent ) :
    DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP( aParent )
{
#ifndef KICAD_SPICE
    spiceFieldsButton->Hide();
#endif /* not KICAD_SPICE */

    m_cmp = NULL;
    m_part = NULL;
    m_skipCopyFromPanel = false;

    wxListItem columnLabel;

    columnLabel.SetImage( -1 );

    columnLabel.SetText( _( "Name" ) );
    fieldListCtrl->InsertColumn( 0, columnLabel );

    columnLabel.SetText( _( "Value" ) );
    fieldListCtrl->InsertColumn( 1, columnLabel );

    m_staticTextUnitSize->SetLabel( GetAbbreviatedUnitsLabel( g_UserUnit ) );
    m_staticTextUnitPosX->SetLabel( GetAbbreviatedUnitsLabel( g_UserUnit ) );
    m_staticTextUnitPosY->SetLabel( GetAbbreviatedUnitsLabel( g_UserUnit ) );

    wxToolTip::Enable( true );
    stdDialogButtonSizerOK->SetDefault();

    // Configure button logos
    addFieldButton->SetBitmap( KiBitmap( plus_xpm ) );
    deleteFieldButton->SetBitmap( KiBitmap( minus_xpm ) );
    moveUpButton->SetBitmap( KiBitmap( go_up_xpm ) );
    moveDownButton->SetBitmap( KiBitmap( go_down_xpm ) );

    Fit();
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnListItemDeselected( wxListEvent& event )
{
    if( !m_skipCopyFromPanel )
    {
        if( !copyPanelToSelectedField() )
            event.Skip();   // do not go to the next row
    }
}

void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnTestChipName( wxCommandEvent& event )
{
    LIB_ID id;
    wxString msg;
    wxString partname = chipnameTextCtrl->GetValue();

    if( id.Parse( partname ) != -1 || !id.IsValid() )
    {
        msg.Printf( _( "\"%s\" is not a valid library symbol indentifier." ), partname );
        DisplayError( this, msg );
        return;
    }

    LIB_ALIAS* alias = NULL;

    try
    {
        alias = Prj().SchSymbolLibTable()->LoadSymbol( id );
    }
    catch( ... )
    {
    }

    if( !alias )
    {
        msg.Printf( _( "Symbol \"%s\" not found in library \"%s\"" ),
                    id.GetLibItemName().wx_str(), id.GetLibNickname().wx_str() );
        DisplayError( this, msg );
        return;
    }

    msg.Printf( _( "Symbol \"%s\" found in library \"%s\"" ),
                id.GetLibItemName().wx_str(), id.GetLibNickname().wx_str() );

    DisplayInfoMessage( this, msg );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnSelectChipName( wxCommandEvent& event )
{
    SCH_BASE_FRAME::HISTORY_LIST dummy;

    auto sel = GetParent()->SelectComponentFromLibrary( NULL, dummy, true, 0, 0, false );

    if( !sel.LibId.IsValid() )
        return;

    chipnameTextCtrl->SetValue( sel.LibId.Format() );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::EditSpiceModel( wxCommandEvent& event )
{
#ifdef KICAD_SPICE
    setSelectedFieldNdx( 0 );
    DIALOG_SPICE_MODEL dialog( this, *m_cmp, m_FieldsBuf );

    if( dialog.ShowModal() == wxID_OK )
        updateDisplay();
#endif /* KICAD_SPICE */
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnListItemSelected( wxListEvent& event )
{
    // remember the selected row, statically
    s_SelectedRow = event.GetIndex();

    copySelectedFieldToPanel();
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnCloseDialog( wxCloseEvent& event )
{
    // On wxWidgets 2.8, and on Linux, calling EndQuasiModal here is mandatory
    // Otherwise, the main event loop is never restored, and Eeschema does not
    // respond to any event, because the DIALOG_SHIM destructor is never called.
    // On wxWidgets 3.0, or on Windows, the DIALOG_SHIM destructor is called,
    // and calls EndQuasiModal.
    // therefore calling EndQuasiModal here is not always mandatory but it creates no issues
    EndQuasiModal( wxID_CANCEL );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnCancelButtonClick( wxCommandEvent& event )
{
    EndQuasiModal( wxID_CANCEL );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::copyPanelToOptions()
{
    LIB_ID id;
    wxString msg;
    wxString tmp = chipnameTextCtrl->GetValue();

    tmp.Replace( wxT( " " ), wxT( "_" ) );

    id.Parse( tmp );

    // Save current flags which could be modified by next change settings
    STATUS_FLAGS flags = m_cmp->GetFlags();

    if( !id.IsValid() )
    {
        msg.Printf( _( "Symbol library identifier \"%s\" is not valid!" ), tmp );
        DisplayError( this, msg );
    }
    else if( id != m_cmp->GetLibId() )
    {
        LIB_ALIAS* alias = NULL;

        try
        {
            alias = Prj().SchSymbolLibTable()->LoadSymbol( id );
        }
        catch( ... )
        {
        }

        if( !alias )
        {
            msg.Printf( _( "Symbol \"%s\" not found in library \"%s\"!" ),
                        id.GetLibItemName().wx_str(), id.GetLibNickname().wx_str() );
            DisplayError( this, msg );
        }
        else    // Change symbol from lib!
        {
            m_cmp->SetLibId( id, Prj().SchSymbolLibTable(), Prj().SchLibs()->GetCacheLibrary() );
        }
    }

    // For symbols with multiple shapes (De Morgan representation) Set the selected shape:
    if( convertCheckBox->IsEnabled() )
    {
        m_cmp->SetConvert( convertCheckBox->GetValue() ? 2 : 1 );
    }

    //Set the part selection in multiple part per package
    if( m_cmp->GetUnit() )
    {
        int unit_selection = unitChoice->GetCurrentSelection() + 1;

        m_cmp->SetUnitSelection( &GetParent()->GetCurrentSheet(), unit_selection );
        m_cmp->SetUnit( unit_selection );
    }

    switch( orientationRadioBox->GetSelection() )
    {
    case 0:
        m_cmp->SetOrientation( CMP_ORIENT_0 );
        break;

    case 1:
        m_cmp->SetOrientation( CMP_ORIENT_90 );
        break;

    case 2:
        m_cmp->SetOrientation( CMP_ORIENT_180 );
        break;

    case 3:
        m_cmp->SetOrientation( CMP_ORIENT_270 );
        break;
    }

    int mirror = mirrorRadioBox->GetSelection();

    switch( mirror )
    {
    case 0:
        break;

    case 1:
        m_cmp->SetOrientation( CMP_MIRROR_X );
        break;

    case 2:
        m_cmp->SetOrientation( CMP_MIRROR_Y );
        break;
    }

    // Restore m_Flag modified by SetUnit() and other change settings
    m_cmp->ClearFlags();
    m_cmp->SetFlags( flags );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::OnOKButtonClick( wxCommandEvent& event )
{
    bool removeRemainingFields = false;

    if( !copyPanelToSelectedField() )
        return;

    if( ! SCH_COMPONENT::IsReferenceStringValid( m_FieldsBuf[REFERENCE].GetText() ) )
    {
        DisplayError( NULL, _( "Illegal reference. A reference must start with a letter" ) );
        return;
    }

    // save old cmp in undo list if not already in edit, or moving ...
    // or the component to be edited is part of a block
    if( m_cmp->GetFlags() == 0
      || GetParent()->GetScreen()->m_BlockLocate.GetState() != STATE_NO_BLOCK )
        GetParent()->SaveCopyInUndoList( m_cmp, UR_CHANGED );

    copyPanelToOptions();

    // Delete any fields with no name before we copy all of m_FieldsBuf back into the component.
    for( unsigned i = MANDATORY_FIELDS;  i<m_FieldsBuf.size(); )
    {
        if( m_FieldsBuf[i].GetName( false ).IsEmpty() || m_FieldsBuf[i].GetText().IsEmpty() )
        {
            // If a field has no value and is not it the field template list, warn the user
            // that it will be remove from the component.  This gives the user a chance to
            // correct the problem before removing the undefined fields.  It should also
            // resolve most of the bug reports and questions regarding missing fields.
            if( !m_FieldsBuf[i].GetName( false ).IsEmpty() && m_FieldsBuf[i].GetText().IsEmpty()
              && !GetParent()->GetTemplates().HasFieldName( m_FieldsBuf[i].GetName( false ) )
              && !removeRemainingFields )
            {
                wxString msg = wxString::Format(
                    _( "The field name \"%s\" does not have a value and is not defined in "
                       "the field template list.  Empty field values are invalid an will "
                       "be removed from the component.  Do you wish to remove this and "
                       "all remaining undefined fields?" ),
                    GetChars( m_FieldsBuf[i].GetName( false ) )
                    );

                wxMessageDialog dlg( this, msg, _( "Remove Fields" ), wxYES_NO | wxNO_DEFAULT );

                if( dlg.ShowModal() == wxID_NO )
                    return;

                removeRemainingFields = true;
            }

            m_FieldsBuf.erase( m_FieldsBuf.begin() + i );
            continue;
        }

        ++i;
    }

    // change all field positions from relative to absolute
    for( unsigned i = 0;  i<m_FieldsBuf.size();  ++i )
    {
        m_FieldsBuf[i].Offset( m_cmp->m_Pos );
    }

    LIB_PART* entry = GetParent()->GetLibPart( m_cmp->GetLibId() );

    if( entry && entry->IsPower() )
        m_FieldsBuf[VALUE].SetText( m_cmp->GetLibId().GetLibItemName() );

    // copy all the fields back, and change the length of m_Fields.
    m_cmp->SetFields( m_FieldsBuf );

    // Reference has a specific initialization, depending on the current active sheet
    // because for a given component, in a complex hierarchy, there are more than one
    // reference.
    m_cmp->SetRef( &GetParent()->GetCurrentSheet(), m_FieldsBuf[REFERENCE].GetText() );

    // The value, footprint and datasheet fields should be kept in sync in multi-unit
    // parts.
    if( m_cmp->GetUnitCount() > 1 )
    {
        const LIB_ID   thisLibId = m_cmp->GetLibId();
        const wxString thisRef   = m_cmp->GetRef( &( GetParent()->GetCurrentSheet() ) );
        int            thisUnit  = m_cmp->GetUnit();

        SCH_REFERENCE_LIST components;
        GetParent()->GetCurrentSheet().GetComponents( components );
        for( unsigned i = 0; i < components.GetCount(); i++ )
        {
            SCH_REFERENCE component = components[i];
            if( component.GetLibPart()->GetLibId() == thisLibId
                    && component.GetRef() == thisRef
                    && component.GetUnit() != thisUnit )
            {
                SCH_COMPONENT* otherUnit = component.GetComp();
                GetParent()->SaveCopyInUndoList( otherUnit, UR_CHANGED, true /* append */);
                otherUnit->GetField( VALUE )->SetText( m_FieldsBuf[VALUE].GetText() );
                otherUnit->GetField( FOOTPRINT )->SetText( m_FieldsBuf[FOOTPRINT].GetText() );
                otherUnit->GetField( DATASHEET )->SetText( m_FieldsBuf[DATASHEET].GetText() );
            }
        }
    }

    GetParent()->OnModify();
    GetParent()->GetScreen()->TestDanglingEnds();

    EndQuasiModal( wxID_OK );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::addFieldButtonHandler( wxCommandEvent& event )
{
    // in case m_FieldsBuf[REFERENCE].m_Orient has changed on screen only, grab
    // screen contents.
    if( !copyPanelToSelectedField() )
        return;

    unsigned  fieldNdx = m_FieldsBuf.size();

    SCH_FIELD blank( wxPoint(), fieldNdx, m_cmp );

    blank.SetTextAngle( m_FieldsBuf[REFERENCE].GetTextAngle() );

    m_FieldsBuf.push_back( blank );
    m_FieldsBuf[fieldNdx].SetName( TEMPLATE_FIELDNAME::GetDefaultFieldName( fieldNdx ) );

    m_skipCopyFromPanel = true;
    setRowItem( fieldNdx, m_FieldsBuf[fieldNdx] );

    setSelectedFieldNdx( fieldNdx );
    m_skipCopyFromPanel = false;
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::deleteFieldButtonHandler( wxCommandEvent& event )
{
    unsigned fieldNdx = getSelectedFieldNdx();

    if( fieldNdx >= m_FieldsBuf.size() )    // traps the -1 case too
        return;

    if( fieldNdx < MANDATORY_FIELDS )
    {
        wxBell();
        return;
    }

    m_skipCopyFromPanel = true;
    m_FieldsBuf.erase( m_FieldsBuf.begin() + fieldNdx );
    fieldListCtrl->DeleteItem( fieldNdx );

    if( fieldNdx >= m_FieldsBuf.size() )
        --fieldNdx;

    updateDisplay();

    setSelectedFieldNdx( fieldNdx );
    m_skipCopyFromPanel = false;
}

static wxString resolveUriByEnvVars( const wxString& aUri )
{
    // URL-like URI: return as is.
    wxURL url( aUri );
    if( url.GetError() == wxURL_NOERR )
    {
        return aUri;
    }
    // Otherwise, the path points to a local file. Resolve environment
    // variables if any.
    return ExpandEnvVarSubstitutions( aUri );
}

void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::showButtonHandler( wxCommandEvent& event )
{
    unsigned fieldNdx = getSelectedFieldNdx();

    if( fieldNdx == DATASHEET )
    {
        wxString datasheet_uri = fieldValueTextCtrl->GetValue();
        datasheet_uri = resolveUriByEnvVars( datasheet_uri );
        GetAssociatedDocument( this, datasheet_uri );
    }
    else if( fieldNdx == FOOTPRINT )
    {
        // pick a footprint using the footprint picker.
        wxString fpid;

        KIWAY_PLAYER* frame = Kiway().Player( FRAME_PCB_MODULE_VIEWER_MODAL, true, this );

        if( frame->ShowModal( &fpid, this ) )
        {
            // DBG( printf( "%s: %s\n", __func__, TO_UTF8( fpid ) ); )
            fieldValueTextCtrl->SetValue( fpid );

            setRowItem( fieldNdx, m_FieldsBuf[fieldNdx].GetName( false ), fpid );
        }

        frame->Destroy();
    }
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::moveUpButtonHandler( wxCommandEvent& event )
{
    unsigned fieldNdx = getSelectedFieldNdx();

    if( fieldNdx >= m_FieldsBuf.size() )    // traps the -1 case too
        return;

    if( fieldNdx <= MANDATORY_FIELDS )
    {
        wxBell();
        return;
    }

    if( !copyPanelToSelectedField() )
        return;

    // swap the fieldNdx field with the one before it, in both the vector
    // and in the fieldListCtrl
    SCH_FIELD tmp = m_FieldsBuf[fieldNdx - 1];

    DBG( printf( "tmp.m_Text=\"%s\" tmp.m_Name=\"%s\"\n",
                 TO_UTF8( tmp.GetText() ), TO_UTF8( tmp.GetName( false ) ) ); )

    m_FieldsBuf[fieldNdx - 1] = m_FieldsBuf[fieldNdx];
    setRowItem( fieldNdx - 1, m_FieldsBuf[fieldNdx] );

    m_FieldsBuf[fieldNdx] = tmp;
    setRowItem( fieldNdx, tmp );

    updateDisplay();

    m_skipCopyFromPanel = true;
    setSelectedFieldNdx( fieldNdx - 1 );
    m_skipCopyFromPanel = false;
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::moveDownButtonHandler( wxCommandEvent& event )
{
    unsigned fieldNdx = getSelectedFieldNdx();

    // Ensure there is at least one field after this one
    if( fieldNdx >= ( m_FieldsBuf.size() - 1 ) )
    {
        return;
    }

    // The first field which can be moved up is the second user field
    // so any field which id < MANDATORY_FIELDS cannot be moved down
    if( fieldNdx < MANDATORY_FIELDS )
        return;

    if( !copyPanelToSelectedField() )
        return;

    // swap the fieldNdx field with the one before it, in both the vector
    // and in the fieldListCtrl
    SCH_FIELD tmp = m_FieldsBuf[fieldNdx + 1];

    m_FieldsBuf[fieldNdx + 1] = m_FieldsBuf[fieldNdx];
    setRowItem( fieldNdx + 1, m_FieldsBuf[fieldNdx] );
    m_FieldsBuf[fieldNdx + 1].SetId( fieldNdx + 1 );

    m_FieldsBuf[fieldNdx] = tmp;
    setRowItem( fieldNdx, tmp );
    m_FieldsBuf[fieldNdx].SetId( fieldNdx );

    updateDisplay( );

    m_skipCopyFromPanel = true;
    setSelectedFieldNdx( fieldNdx + 1 );
    m_skipCopyFromPanel = false;
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::setSelectedFieldNdx( int aFieldNdx )
{
    /* deselect old selection, but I think this is done by single selection
     * flag within fieldListCtrl.
     * fieldListCtrl->SetItemState( s_SelectedRow, 0,
     *                              wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
     */

    if( aFieldNdx >= (int) m_FieldsBuf.size() )
        aFieldNdx = m_FieldsBuf.size() - 1;

    if( aFieldNdx < 0 )
        aFieldNdx = 0;

    fieldListCtrl->SetItemState( aFieldNdx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    fieldListCtrl->EnsureVisible( aFieldNdx );

    s_SelectedRow = aFieldNdx;
}


int DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::getSelectedFieldNdx()
{
    return s_SelectedRow;
}


SCH_FIELD* DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::findField( const wxString& aFieldName )
{
    for( unsigned i=0;  i<m_FieldsBuf.size();  ++i )
    {
        if( aFieldName == m_FieldsBuf[i].GetName( false ) )
            return &m_FieldsBuf[i];
    }

    return NULL;
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::InitBuffers( SCH_COMPONENT* aComponent )
{
    m_cmp = aComponent;

    /*  We have 3 component related field lists to be aware of: 1) UI
        presentation, 2) fields in component ram copy, and 3) fields recorded
        with component on disk. m_FieldsBuf is the list of UI fields, and this
        list is not the same as the list which is in the component, which is
        also not the same as the list on disk. All 3 lists are potentially
        different. In the UI we choose to preserve the order of the first
        MANDATORY_FIELDS which are sometimes called fixed fields. Then we append
        the template fieldnames in the exact same order as the template
        fieldname editor shows them. Then we append any user defined fieldnames
        which came from the component.
    */

    m_part = GetParent()->GetLibPart( m_cmp->GetLibId(), true );

#if 0 && defined(DEBUG)
    for( int i = 0;  i<aComponent->GetFieldCount();  ++i )
    {
        printf( "Orig[%d] (x=%d, y=%d)\n", i,
                aComponent->m_Fields[i].GetTextPos().x,
                aComponent->m_Fields[i].GetTextPos().y );
    }

#endif

    // When this code was written, all field constructors ensure that the fixed fields
    // are all present within a component.  So we can knowingly copy them over
    // in the normal order.  Copy only the fixed fields at first.
    // Please do not break the field constructors.

    m_FieldsBuf.clear();

    for( int i=0;  i<MANDATORY_FIELDS;  ++i )
    {
        m_FieldsBuf.push_back(  aComponent->m_Fields[i] );

        // make the editable field position relative to the component
        m_FieldsBuf[i].Offset( -m_cmp->m_Pos );

        // Ensure the Field name reflects the default name, even if the
        // local has changed since schematic was read
        m_FieldsBuf[i].SetName( TEMPLATE_FIELDNAME::GetDefaultFieldName( i ) );
    }

    // Add template fieldnames:
    // Now copy in the template fields, in the order that they are present in the
    // template field editor UI.
    const TEMPLATE_FIELDNAMES& tfnames = GetParent()->GetTemplateFieldNames();

    for( TEMPLATE_FIELDNAMES::const_iterator it = tfnames.begin();  it!=tfnames.end();  ++it )
    {
        // add a new field unconditionally to the UI only
        SCH_FIELD   fld( wxPoint(0,0), -1 /* id is a relic */, m_cmp, it->m_Name );

        // See if field by same name already exists in component.
        SCH_FIELD* schField = aComponent->FindField( it->m_Name );

        // If the field does not already exist in the component, then we
        // use defaults from the template fieldname, otherwise the original
        // values from the component will be set.
        if( !schField )
        {
            if( !it->m_Visible )
                fld.SetVisible( false );
            else
                fld.SetVisible( true );

            fld.SetText( it->m_Value );   // empty? ok too.
        }
        else
        {
            fld = *schField;

            // make the editable field position relative to the component
            fld.Offset( -m_cmp->m_Pos );
        }

        m_FieldsBuf.push_back( fld );
    }

    // Lastly, append any original fields from the component which were not added
    // from the set of fixed fields nor from the set of template fields.
    for( unsigned i=MANDATORY_FIELDS;  i<aComponent->m_Fields.size();  ++i )
    {
        SCH_FIELD*  cmp = &aComponent->m_Fields[i];
        SCH_FIELD*  buf = findField( cmp->GetName( false ) );

        if( !buf )
        {
            int newNdx = m_FieldsBuf.size();
            m_FieldsBuf.push_back( *cmp );

            // make the editable field position relative to the component
            m_FieldsBuf[newNdx].Offset( -m_cmp->m_Pos );
        }
    }


#if 0 && defined(DEBUG)
    for( unsigned i = 0;  i<m_FieldsBuf.size();  ++i )
    {
        printf( "m_FieldsBuf[%d] (x=%-3d, y=%-3d) name:%s\n", i, m_FieldsBuf[i].m_Pos.x,
                m_FieldsBuf[i].m_Pos.y, TO_UTF8(m_FieldsBuf[i].GetName( false ) ) );
    }
#endif

    m_FieldsBuf[REFERENCE].SetText( m_cmp->GetRef( &GetParent()->GetCurrentSheet() ) );

    for( unsigned i = 0;  i<m_FieldsBuf.size();  ++i )
    {
        setRowItem( i, m_FieldsBuf[i] );
    }

#if 0 && defined(DEBUG)
    for( unsigned i = 0;  i<m_FieldsBuf.size();  ++i )
    {
        printf( "after[%d] (x=%d, y=%d)\n", i, m_FieldsBuf[i].m_Pos.x,
                m_FieldsBuf[i].m_Pos.y );
    }

#endif

    copyOptionsToPanel();

    // disable some options inside the edit dialog
    // which can cause problems while dragging
    if( m_cmp->IsDragging() )
    {
        orientationRadioBox->Disable();
        mirrorRadioBox->Disable();
        chipnameTextCtrl->Disable();
    }

    // put focus on the list ctrl
    fieldListCtrl->SetFocus();

    // resume editing at the last row edited, last time dialog was up.
    setSelectedFieldNdx( s_SelectedRow );

    copySelectedFieldToPanel();
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::setRowItem( int aFieldNdx, const wxString& aName, const wxString& aValue )
{
    wxASSERT( aFieldNdx >= 0 );

    // insert blanks if aFieldNdx is referencing a "yet to be defined" row
    while( aFieldNdx >= fieldListCtrl->GetItemCount() )
    {
        long ndx = fieldListCtrl->InsertItem( fieldListCtrl->GetItemCount(), wxEmptyString );

        wxASSERT( ndx >= 0 );

        fieldListCtrl->SetItem( ndx, 1, wxEmptyString );
    }

    fieldListCtrl->SetItem( aFieldNdx, 0, aName );
    fieldListCtrl->SetItem( aFieldNdx, 1, aValue );

    // recompute the column widths here, after setting texts
    fieldListCtrl->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    fieldListCtrl->SetColumnWidth( 1, wxLIST_AUTOSIZE );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::copySelectedFieldToPanel()
{
    wxCHECK_RET( m_cmp != NULL, wxT( "Component pointer not initialized." ) );

    unsigned fieldNdx = getSelectedFieldNdx();

    if( fieldNdx >= m_FieldsBuf.size() )    // traps the -1 case too
        return;

    SCH_FIELD& field = m_FieldsBuf[fieldNdx];

    showCheckBox->SetValue( field.IsVisible() );

    rotateCheckBox->SetValue( field.GetTextAngle() == TEXT_ANGLE_VERT );

    int style = 0;

    if( field.IsItalic() )
        style = 1;

    if( field.IsBold() )
        style |= 2;

    m_StyleRadioBox->SetSelection( style );

    // Select the right text justification
    if( field.GetHorizJustify() == GR_TEXT_HJUSTIFY_LEFT )
        m_FieldHJustifyCtrl->SetSelection( 0 );
    else if( field.GetHorizJustify() == GR_TEXT_HJUSTIFY_RIGHT )
        m_FieldHJustifyCtrl->SetSelection( 2 );
    else
        m_FieldHJustifyCtrl->SetSelection( 1 );

    if( field.GetVertJustify() == GR_TEXT_VJUSTIFY_TOP )
        m_FieldVJustifyCtrl->SetSelection( 0 );
    else if( field.GetVertJustify() == GR_TEXT_VJUSTIFY_BOTTOM )
        m_FieldVJustifyCtrl->SetSelection( 2 );
    else
        m_FieldVJustifyCtrl->SetSelection( 1 );


    fieldNameTextCtrl->SetValue( field.GetName( false ) );

    // the names of the fixed fields are not editable, others are.
    //fieldNameTextCtrl->Enable(  fieldNdx >= MANDATORY_FIELDS );
    fieldNameTextCtrl->SetEditable( fieldNdx >= MANDATORY_FIELDS );

    // only user defined fields may be moved, and not the top most user defined
    // field since it would be moving up into the fixed fields, > not >=
    moveUpButton->Enable( fieldNdx > MANDATORY_FIELDS );
    moveDownButton->Enable( ( fieldNdx >= MANDATORY_FIELDS ) && ( fieldNdx < ( m_FieldsBuf.size() - 1 ) ) );

    // may only delete user defined fields
    //deleteFieldButton->Enable( fieldNdx >= MANDATORY_FIELDS );
    
    fieldValueTextCtrl->SetValidator( SCH_FIELD_VALIDATOR( false, field.GetId() ) );
    fieldValueTextCtrl->SetValue( field.GetText() );

    m_show_datasheet_button->Enable( fieldNdx == DATASHEET || fieldNdx == FOOTPRINT );

    if( fieldNdx == DATASHEET )
    {
        m_show_datasheet_button->SetLabel( _( "Show Datasheet" ) );
        m_show_datasheet_button->SetToolTip(
            _("If your datasheet is given as an http:// link,"
              " then pressing this button should bring it up in your webbrowser.") );
    }
    else if( fieldNdx == FOOTPRINT )
    {
        m_show_datasheet_button->SetLabel( _( "Browse Footprints" ) );
        m_show_datasheet_button->SetToolTip(
            _("Open the footprint browser to choose a footprint and assign it.") );
    }
    else
    {
        m_show_datasheet_button->SetLabel( wxEmptyString );
        m_show_datasheet_button->SetToolTip(
            _("Used only for fields Footprint and Datasheet.") );
    }

    // For power symbols, the value is NOR editable, because value and pin
    // name must be same and can be edited only in library editor
    /*
    if( fieldNdx == VALUE && m_part && m_part->IsPower() )
        fieldValueTextCtrl->Enable( false );
    else
        fieldValueTextCtrl->Enable( true );
    */
    textSizeTextCtrl->SetValue( EDA_GRAPHIC_TEXT_CTRL::FormatSize( g_UserUnit, field.GetTextWidth() ) );

    // disable all editing of fields because they should be edited in library editor
    fieldNameTextCtrl->Enable(  false );
    deleteFieldButton->Enable( false );
    fieldValueTextCtrl->Enable( false );


    wxPoint coord = field.GetTextPos();
    wxPoint zero  = -m_cmp->m_Pos;  // relative zero

    // If the field value is empty and the position is at relative zero, we
    // set the initial position as a small offset from the ref field, and
    // orient it the same as the ref field.  That is likely to put it at least
    // close to the desired position.
    if( coord == zero && field.GetText().IsEmpty() )
    {
        rotateCheckBox->SetValue( m_FieldsBuf[REFERENCE].GetTextAngle() == TEXT_ANGLE_VERT );

        coord.x = m_FieldsBuf[REFERENCE].GetTextPos().x
            + ( fieldNdx - MANDATORY_FIELDS + 1 ) * 100;

        coord.y = m_FieldsBuf[REFERENCE].GetTextPos().y
            + ( fieldNdx - MANDATORY_FIELDS + 1 ) * 100;

        // coord can compute negative if field is < MANDATORY_FIELDS, e.g. FOOTPRINT.
        // That is ok, we basically don't want all the new empty fields on
        // top of each other.
    }

    wxString coordText = StringFromValue( g_UserUnit, coord.x );
    posXTextCtrl->SetValue( coordText );

    coordText = StringFromValue( g_UserUnit, coord.y );
    posYTextCtrl->SetValue( coordText );
}


bool DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::copyPanelToSelectedField()
{
    unsigned fieldNdx = getSelectedFieldNdx();

    if( fieldNdx >= m_FieldsBuf.size() )        // traps the -1 case too
        return true;

    // Check for illegal field text.
    if( fieldValueTextCtrl->GetValidator()
      && !fieldValueTextCtrl->GetValidator()->Validate( this ) )
        return false;

    SCH_FIELD& field = m_FieldsBuf[fieldNdx];

    field.SetVisible( showCheckBox->GetValue() );

    if( rotateCheckBox->GetValue() )
        field.SetTextAngle( TEXT_ANGLE_VERT );
    else
        field.SetTextAngle( TEXT_ANGLE_HORIZ );

    rotateCheckBox->SetValue( field.GetTextAngle() == TEXT_ANGLE_VERT );

    // Copy the text justification
    static const EDA_TEXT_HJUSTIFY_T hjustify[] = {
        GR_TEXT_HJUSTIFY_LEFT,
        GR_TEXT_HJUSTIFY_CENTER,
        GR_TEXT_HJUSTIFY_RIGHT
    };

    static const EDA_TEXT_VJUSTIFY_T vjustify[] = {
        GR_TEXT_VJUSTIFY_TOP,
        GR_TEXT_VJUSTIFY_CENTER,
        GR_TEXT_VJUSTIFY_BOTTOM
    };

    field.SetHorizJustify( hjustify[m_FieldHJustifyCtrl->GetSelection()] );
    field.SetVertJustify( vjustify[m_FieldVJustifyCtrl->GetSelection()] );

    field.SetName( fieldNameTextCtrl->GetValue() );

    /* Void fields texts for REFERENCE and VALUE (value is the name of the
     * component in lib ! ) are not allowed
     * change them only for a new non void value
     * When void, usually netlists are broken
     */
    if( !fieldValueTextCtrl->GetValue().IsEmpty() || fieldNdx > VALUE )
        field.SetText( fieldValueTextCtrl->GetValue() );

    setRowItem( fieldNdx, field );  // update fieldListCtrl

    int tmp = EDA_GRAPHIC_TEXT_CTRL::ParseSize( textSizeTextCtrl->GetValue(), g_UserUnit );
    field.SetTextSize( wxSize( tmp, tmp ) );
    int style = m_StyleRadioBox->GetSelection();

    field.SetItalic( (style & 1 ) != 0 );
    field.SetBold( (style & 2 ) != 0 );

    wxPoint pos;
    pos.x = ValueFromString( g_UserUnit, posXTextCtrl->GetValue() );
    pos.y = ValueFromString( g_UserUnit, posYTextCtrl->GetValue() );
    field.SetTextPos( pos );

    return true;
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::copyOptionsToPanel()
{
    // Remove non existing choices (choiceCount must be <= number for parts)
    int unitcount = m_part ? m_part->GetUnitCount() : 1;

    if( unitcount < 1 )
        unitcount = 1;

    unitChoice->Clear();

    for( int ii = 1; ii <= unitcount; ii++ )
    {
        unitChoice->Append( LIB_PART::SubReference(  ii, false ) );
    }

    // For symbols with multiple parts per package, set the unit selection
    if( m_cmp->GetUnit() <= (int)unitChoice->GetCount() )
        unitChoice->SetSelection( m_cmp->GetUnit() - 1 );

    // Disable unit selection if only one unit exists:
    if( m_cmp->GetUnitCount() <= 1 )
    {
        unitChoice->Enable( false );
        unitsInterchageableLabel->Show( false );
        unitsInterchageableText->Show( false );
    }
    else
    {
        // Show the "Units are not interchangeable" message option?
        if( !m_part || !m_part->UnitsLocked() )
            unitsInterchageableLabel->SetLabel( _( "Yes" ) );
        else
            unitsInterchageableLabel->SetLabel( _( "No" ) );
    }

    int orientation = m_cmp->GetOrientation() & ~( CMP_MIRROR_X | CMP_MIRROR_Y );

    if( orientation == CMP_ORIENT_90 )
        orientationRadioBox->SetSelection( 1 );
    else if( orientation == CMP_ORIENT_180 )
        orientationRadioBox->SetSelection( 2 );
    else if( orientation == CMP_ORIENT_270 )
        orientationRadioBox->SetSelection( 3 );
    else
        orientationRadioBox->SetSelection( 0 );

    int mirror = m_cmp->GetOrientation() & ( CMP_MIRROR_X | CMP_MIRROR_Y );

    if( mirror == CMP_MIRROR_X )
    {
        mirrorRadioBox->SetSelection( 1 );
        DBG( printf( "mirror=X,1\n" ); )
    }
    else if( mirror == CMP_MIRROR_Y )
    {
        mirrorRadioBox->SetSelection( 2 );
        DBG( printf( "mirror=Y,2\n" ); )
    }
    else
        mirrorRadioBox->SetSelection( 0 );

    // Activate/Desactivate the normal/convert option ? (activated only if
    // the component has more than one shape)
    if( m_cmp->GetConvert() > 1 )
        convertCheckBox->SetValue( true );

    if( m_part == NULL || !m_part->HasConversion() )
        convertCheckBox->Enable( false );

    // Set the component's library name.
    chipnameTextCtrl->SetValue( m_cmp->GetLibId().Format() );

    // Set the component's unique ID time stamp.
    m_textCtrlTimeStamp->SetValue( wxString::Format( wxT( "%8.8lX" ),
                                   (unsigned long) m_cmp->GetTimeStamp() ) );
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::SetInitCmp( wxCommandEvent& event )
{
    if( !m_cmp )
        return;

    if( LIB_PART* part = GetParent()->GetLibPart( m_cmp->GetLibId()  ) )
    {
        // save old cmp in undo list if not already in edit, or moving ...
        if( m_cmp->GetFlags() == 0 )
            GetParent()->SaveCopyInUndoList( m_cmp, UR_CHANGED );

        INSTALL_UNBUFFERED_DC( dc, GetParent()->GetCanvas() );
        m_cmp->Draw( GetParent()->GetCanvas(), &dc, wxPoint( 0, 0 ), g_XorMode );

        // Initialize fixed field values to default values found in library
        // Note: the field texts are not modified because they are set in schematic,
        // the text from libraries is most of time a dummy text
        // Only VALUE, REFERENCE , FOOTPRINT and DATASHEET are re-initialized
        LIB_FIELD& refField = part->GetReferenceField();

        m_cmp->GetField( REFERENCE )->ImportValues( refField );
        m_cmp->GetField( REFERENCE )->SetTextPos( refField.GetTextPos() + m_cmp->m_Pos );

        LIB_FIELD& valField = part->GetValueField();

        m_cmp->GetField( VALUE )->ImportValues( valField );
        m_cmp->GetField( VALUE )->SetTextPos( valField.GetTextPos() + m_cmp->m_Pos );

        LIB_FIELD* field = part->GetField(FOOTPRINT);

        if( field && m_cmp->GetField( FOOTPRINT ) )
        {
            m_cmp->GetField( FOOTPRINT )->ImportValues( *field );
            m_cmp->GetField( FOOTPRINT )->SetTextPos( field->GetTextPos() + m_cmp->m_Pos );
        }

        field = part->GetField(DATASHEET);

        if( field && m_cmp->GetField( DATASHEET ) )
        {
            m_cmp->GetField( DATASHEET )->ImportValues( *field );
            m_cmp->GetField( DATASHEET )->SetTextPos( field->GetTextPos() + m_cmp->m_Pos );
        }

        m_cmp->SetOrientation( CMP_NORMAL );

        GetParent()->OnModify();

        m_cmp->Draw( GetParent()->GetCanvas(), &dc, wxPoint( 0, 0 ), GR_DEFAULT_DRAWMODE );

        EndQuasiModal( wxID_OK );
    }
}


void DIALOG_EDIT_COMPONENT_IN_SCHEMATIC::UpdateFields( wxCommandEvent& event )
{
    SCH_COMPONENT copy( *m_cmp );
    std::list<SCH_COMPONENT*> components;
    components.push_back( &copy );
    InvokeDialogUpdateFields( GetParent(), components, false );

    // Copy fields from the modified component copy to the dialog buffer
    m_FieldsBuf.clear();

    for( int i = 0; i < copy.GetFieldCount(); ++i )
    {
        copy.m_Fields[i].SetParent( m_cmp );
        m_FieldsBuf.push_back( copy.m_Fields[i] );
        m_FieldsBuf[i].Offset( -m_cmp->m_Pos );
    }

    // Update the selected field as well
    copySelectedFieldToPanel();
    updateDisplay();
}
