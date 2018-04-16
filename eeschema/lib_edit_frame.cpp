/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@gmail.com>
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

/**
 * @file lib_edit_frame.cpp
 * @brief LIB_EDIT_FRAME class is the symbol library editor frame.
 */

#include <fctsys.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <class_drawpanel.h>
#include <base_screen.h>
#include <confirm.h>
#include <eda_doc.h>
#include <gr_basic.h>
#include <sch_edit_frame.h>
#include <msgpanel.h>
#include <confirm.h>

#include <general.h>
#include <eeschema_id.h>
#include <lib_edit_frame.h>
#include <class_library.h>
#include <lib_polyline.h>
#include <lib_pin.h>

#include <lib_manager.h>
#include <widgets/cmp_tree_pane.h>
#include <widgets/component_tree.h>
#include <widgets/PartPropertiesPane.h>
#include <symbol_lib_table.h>

#include <kicad_device_context.h>
#include <hotkeys.h>
#include <eeschema_config.h>

#include <dialogs/dialog_lib_edit_text.h>
#include <dialogs/dialog_edit_component_in_lib.h>
#include <dialogs/dialog_lib_edit_pin_table.h>

#include <wildcards_and_files_ext.h>

#include <menus_helpers.h>
#include <wx/progdlg.h>


wxString LIB_EDIT_FRAME::      m_aliasName;
int LIB_EDIT_FRAME::           m_unit    = 1;
int LIB_EDIT_FRAME::           m_convert = 1;
LIB_ITEM* LIB_EDIT_FRAME::m_lastDrawItem = NULL;

bool LIB_EDIT_FRAME::          m_showDeMorgan    = false;
wxSize LIB_EDIT_FRAME::        m_clientSize      = wxSize( -1, -1 );
int LIB_EDIT_FRAME::           m_textSize        = -1;
double LIB_EDIT_FRAME::        m_current_text_angle = TEXT_ANGLE_HORIZ;
int LIB_EDIT_FRAME::           m_drawLineWidth   = 0;

// these values are overridden when reading the config
int LIB_EDIT_FRAME::           m_textPinNumDefaultSize = DEFAULTPINNUMSIZE;
int LIB_EDIT_FRAME::           m_textPinNameDefaultSize = DEFAULTPINNAMESIZE;
int LIB_EDIT_FRAME::           m_defaultPinLength = DEFAULTPINLENGTH;

FILL_T LIB_EDIT_FRAME::        m_drawFillStyle   = NO_FILL;


BEGIN_EVENT_TABLE( LIB_EDIT_FRAME, EDA_DRAW_FRAME )
    EVT_CLOSE( LIB_EDIT_FRAME::OnCloseWindow )
    EVT_SIZE( LIB_EDIT_FRAME::OnSize )
    EVT_ACTIVATE( LIB_EDIT_FRAME::OnActivate )

    // Library actions
    EVT_TOOL( ID_LIBEDIT_NEW_LIBRARY, LIB_EDIT_FRAME::OnCreateNewLibrary )
    EVT_TOOL( ID_LIBEDIT_ADD_LIBRARY, LIB_EDIT_FRAME::OnAddLibrary )
    EVT_TOOL( ID_LIBEDIT_SAVE_LIBRARY, LIB_EDIT_FRAME::OnSaveLibrary )
    EVT_MENU( ID_LIBEDIT_SAVE_LIBRARY_AS, LIB_EDIT_FRAME::OnSaveLibrary )
    EVT_MENU( ID_LIBEDIT_SAVE_ALL_LIBS, LIB_EDIT_FRAME::OnSaveAllLibraries )
    EVT_TOOL( ID_LIBEDIT_REVERT_LIBRARY, LIB_EDIT_FRAME::OnRevertLibrary )

    // Part actions
    EVT_TOOL( ID_LIBEDIT_NEW_PART, LIB_EDIT_FRAME::OnCreateNewPart )
    EVT_TOOL( ID_LIBEDIT_EDIT_PART, LIB_EDIT_FRAME::OnEditPart )
    EVT_TOOL( ID_LIBEDIT_IMPORT_PART, LIB_EDIT_FRAME::OnImportPart )
    EVT_TOOL( ID_LIBEDIT_EXPORT_PART, LIB_EDIT_FRAME::OnExportPart )
    EVT_TOOL( ID_LIBEDIT_SAVE_PART, LIB_EDIT_FRAME::OnSavePart )
    EVT_TOOL( ID_LIBEDIT_REVERT_PART, LIB_EDIT_FRAME::OnRevertPart )
    EVT_TOOL( ID_LIBEDIT_REMOVE_PART, LIB_EDIT_FRAME::OnRemovePart )
    EVT_TOOL( ID_LIBEDIT_CUT_PART, LIB_EDIT_FRAME::OnCopyCutPart )
    EVT_TOOL( ID_LIBEDIT_COPY_PART, LIB_EDIT_FRAME::OnCopyCutPart )
    EVT_TOOL( ID_LIBEDIT_PASTE_PART, LIB_EDIT_FRAME::OnPasteDuplicatePart )
    EVT_TOOL( ID_LIBEDIT_DUPLICATE_PART, LIB_EDIT_FRAME::OnPasteDuplicatePart )

    // Main horizontal toolbar.
    EVT_TOOL( ID_TO_LIBVIEW, LIB_EDIT_FRAME::OnOpenLibraryViewer )
    EVT_TOOL( wxID_COPY, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_PASTE, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_CUT, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_UNDO, LIB_EDIT_FRAME::GetComponentFromUndoList )
    EVT_TOOL( wxID_REDO, LIB_EDIT_FRAME::GetComponentFromRedoList )
    EVT_TOOL( ID_LIBEDIT_GET_FRAME_EDIT_PART, LIB_EDIT_FRAME::OnEditComponentProperties )
    EVT_TOOL( ID_LIBEDIT_GET_FRAME_EDIT_FIELDS, LIB_EDIT_FRAME::InstallFieldsEditorDialog )
    EVT_TOOL( ID_LIBEDIT_CHECK_PART, LIB_EDIT_FRAME::OnCheckComponent )
    EVT_TOOL( ID_DE_MORGAN_NORMAL_BUTT, LIB_EDIT_FRAME::OnSelectBodyStyle )
    EVT_TOOL( ID_DE_MORGAN_CONVERT_BUTT, LIB_EDIT_FRAME::OnSelectBodyStyle )
    EVT_TOOL( ID_LIBEDIT_VIEW_DOC, LIB_EDIT_FRAME::OnViewEntryDoc )
    EVT_TOOL( ID_LIBEDIT_SYNC_PIN_EDIT, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_LIBEDIT_EDIT_PIN_BY_TABLE, LIB_EDIT_FRAME::OnOpenPinTable )

    EVT_COMBOBOX( ID_LIBEDIT_SELECT_PART_NUMBER, LIB_EDIT_FRAME::OnSelectPart )
    EVT_COMBOBOX( ID_LIBEDIT_SELECT_ALIAS, LIB_EDIT_FRAME::OnSelectAlias )

    // Right vertical toolbar.
    EVT_TOOL( ID_NO_TOOL_SELECTED, LIB_EDIT_FRAME::OnSelectTool )
    EVT_TOOL( ID_ZOOM_SELECTION, LIB_EDIT_FRAME::OnSelectTool )
    EVT_TOOL_RANGE( ID_LIBEDIT_PIN_BUTT, ID_LIBEDIT_DELETE_ITEM_BUTT,
                    LIB_EDIT_FRAME::OnSelectTool )

    // Left vertical toolbar (option toolbar).
    EVT_TOOL( ID_LIBEDIT_SHOW_ELECTRICAL_TYPE, LIB_EDIT_FRAME::OnShowElectricalType )
    EVT_TOOL( ID_LIBEDIT_SHOW_HIDE_SEARCH_TREE, LIB_EDIT_FRAME::OnToggleSearchTree )

    // menubar commands
    EVT_MENU( wxID_EXIT, LIB_EDIT_FRAME::CloseWindow )
    EVT_MENU( ID_LIBEDIT_GEN_PNG_FILE, LIB_EDIT_FRAME::OnPlotCurrentComponent )
    EVT_MENU( ID_LIBEDIT_GEN_SVG_FILE, LIB_EDIT_FRAME::OnPlotCurrentComponent )
    EVT_MENU( wxID_HELP, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_INDEX, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( ID_HELP_GET_INVOLVED, EDA_DRAW_FRAME::GetKicadContribute )
    EVT_MENU( wxID_ABOUT, EDA_BASE_FRAME::GetKicadAbout )

    EVT_MENU( wxID_PREFERENCES, LIB_EDIT_FRAME::OnPreferencesOptions )

    // Multiple item selection context menu commands.
    EVT_MENU_RANGE( ID_SELECT_ITEM_START, ID_SELECT_ITEM_END, LIB_EDIT_FRAME::OnSelectItem )

    EVT_MENU_RANGE( ID_PREFERENCES_HOTKEY_START, ID_PREFERENCES_HOTKEY_END,
                    LIB_EDIT_FRAME::Process_Config )

    // Context menu events and commands.
    EVT_MENU( ID_LIBEDIT_EDIT_PIN, LIB_EDIT_FRAME::OnEditPin )
    EVT_MENU( ID_LIBEDIT_ROTATE_ITEM, LIB_EDIT_FRAME::OnRotateItem )

    EVT_MENU_RANGE( ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_ITEM,
                    ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT,
                    LIB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
                    LIB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_LIBEDIT_MIRROR_X, ID_LIBEDIT_ORIENT_NORMAL,
                    LIB_EDIT_FRAME::OnOrient )

    // Update user interface elements.
    EVT_UPDATE_UI( wxID_PASTE, LIB_EDIT_FRAME::OnUpdatePaste )
    EVT_UPDATE_UI( ID_LIBEDIT_REVERT_LIBRARY, LIB_EDIT_FRAME::OnUpdateLibModified )
    EVT_UPDATE_UI( ID_LIBEDIT_EXPORT_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_PART, LIB_EDIT_FRAME::OnUpdatePartModified )
    EVT_UPDATE_UI( ID_LIBEDIT_REVERT_PART, LIB_EDIT_FRAME::OnUpdatePartModified )
    EVT_UPDATE_UI( ID_LIBEDIT_PASTE_PART, LIB_EDIT_FRAME::OnUpdateClipboardNotEmpty )
    EVT_UPDATE_UI( ID_LIBEDIT_GET_FRAME_EDIT_FIELDS, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_CHECK_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_GET_FRAME_EDIT_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( wxID_UNDO, LIB_EDIT_FRAME::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, LIB_EDIT_FRAME::OnUpdateRedo )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_LIBRARY, LIB_EDIT_FRAME::OnUpdateSaveLib )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_LIBRARY_AS, LIB_EDIT_FRAME::OnUpdateSaveLibAs )
    EVT_UPDATE_UI( ID_LIBEDIT_VIEW_DOC, LIB_EDIT_FRAME::OnUpdateViewDoc )
    EVT_UPDATE_UI( ID_LIBEDIT_SYNC_PIN_EDIT, LIB_EDIT_FRAME::OnUpdateSyncPinEdit )
    EVT_UPDATE_UI( ID_LIBEDIT_EDIT_PIN_BY_TABLE, LIB_EDIT_FRAME::OnUpdatePinTable )
    EVT_UPDATE_UI( ID_LIBEDIT_SELECT_PART_NUMBER, LIB_EDIT_FRAME::OnUpdatePartNumber )
    EVT_UPDATE_UI( ID_LIBEDIT_SELECT_ALIAS, LIB_EDIT_FRAME::OnUpdateSelectAlias )
    EVT_UPDATE_UI( ID_DE_MORGAN_NORMAL_BUTT, LIB_EDIT_FRAME::OnUpdateDeMorganNormal )
    EVT_UPDATE_UI( ID_DE_MORGAN_CONVERT_BUTT, LIB_EDIT_FRAME::OnUpdateDeMorganConvert )
    EVT_UPDATE_UI( ID_NO_TOOL_SELECTED, LIB_EDIT_FRAME::OnUpdateSelectTool )
    EVT_UPDATE_UI( ID_ZOOM_SELECTION, LIB_EDIT_FRAME::OnUpdateSelectTool )
    EVT_UPDATE_UI_RANGE( ID_LIBEDIT_PIN_BUTT, ID_LIBEDIT_DELETE_ITEM_BUTT,
                         LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_SHOW_ELECTRICAL_TYPE, LIB_EDIT_FRAME::OnUpdateElectricalType )

END_EVENT_TABLE()

#define LIB_EDIT_FRAME_NAME wxT( "LibeditFrame" )

LIB_EDIT_FRAME::LIB_EDIT_FRAME( KIWAY* aKiway, wxWindow* aParent ) :
    SCH_BASE_FRAME( aKiway, aParent, FRAME_SCH_LIB_EDITOR, _( "Library Editor" ),
        wxDefaultPosition, wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, LIB_EDIT_FRAME_NAME )
{
    m_showAxis   = true;            // true to draw axis
    SetShowDeMorgan( false );
    m_drawSpecificConvert = true;
    m_drawSpecificUnit    = false;
    m_hotkeysDescrList    = g_Libedit_Hokeys_Descr;
    m_syncPinEdit         = false;
    m_repeatPinStep = DEFAULT_REPEAT_OFFSET_PIN;
    SetShowElectricalType( true );

    m_my_part = NULL;
    m_tempCopyComponent = NULL;
    m_treePane = nullptr;
    m_libMgr = nullptr;

    // Delayed initialization
    if( m_textSize == -1 )
        m_textSize = GetDefaultTextSize();

    // Initialize grid id to the default value 50 mils:
    m_LastGridSizeId = ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000;

    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( icon_libedit_xpm ) );
    SetIcon( icon );

    LoadSettings( config() );

    m_dummyScreen = new SCH_SCREEN( aKiway );
    SetScreen( m_dummyScreen );
    GetScreen()->m_Center = true;
    GetScreen()->SetMaxUndoItems( m_UndoRedoCountMax );

    SetCrossHairPosition( wxPoint( 0, 0 ) );

    // Ensure m_LastGridSizeId is an offset inside the allowed schematic range
    if( m_LastGridSizeId < ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000 )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000;

    if( m_LastGridSizeId > ID_POPUP_GRID_LEVEL_1 - ID_POPUP_GRID_LEVEL_1000 )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_1 - ID_POPUP_GRID_LEVEL_1000;

    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    if( m_canvas )
        m_canvas->SetEnableBlockCommands( true );

    m_libMgr = new LIB_MANAGER( *this );
    SyncLibraries( true );
    m_treePane = new CMP_TREE_PANE( this, m_libMgr );

    m_propsPane = new PartPropertiesPane(this);

    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateVToolbar();

    // Ensure the current alias name is valid if a part is loaded
    // Sometimes it is not valid. This is the case
    // when a part value (the part lib name), or the alias list was modified
    // during a previous session and the modifications not saved in lib.
    // Reopen libedit in a new session gives a non valid m_aliasName
    // because the curr part is reloaded from the library (and this is the unmodified part)
    // and the old alias name (from the previous edition) can be invalid
    LIB_PART* part = GetCurPart();

    if( part == NULL )
        m_aliasName.Empty();
    else if( m_aliasName != part->GetName() )
    {
        LIB_ALIAS* alias = part->GetAlias( m_aliasName );

        if( !alias )
            m_aliasName = part->GetName();
    }

    ReCreateOptToolbar();
    DisplayLibInfos();
    DisplayCmpDoc();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    m_auimgr.SetManagedWindow( this );

    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO vert;
    vert.VerticalToolbarPane();

    EDA_PANEINFO mesg;
    mesg.MessageToolbarPane();

    m_auimgr.AddPane( m_mainToolBar,
                      wxAuiPaneInfo( horiz ).Name( "m_mainToolBar" ).Top().Row( 0 ) );

    m_auimgr.AddPane( m_drawToolBar,
                      wxAuiPaneInfo( vert ).Name( "m_VToolBar" ).Right() );

    m_auimgr.AddPane( m_optionsToolBar,
                      wxAuiPaneInfo( vert ).Name( "m_optionsToolBar" ).Left().Row( 0 ) );

    m_auimgr.AddPane( m_canvas,
                      wxAuiPaneInfo().Name( "DrawFrame" ).CentrePane() );

    m_auimgr.AddPane( m_messagePanel,
                      wxAuiPaneInfo( mesg ).Name( "MsgPanel" ).Bottom().Layer( 10 ) );

    m_auimgr.AddPane( m_treePane, wxAuiPaneInfo().Name( "ComponentTree" ).Left().Row( 1 )
            .Resizable().MinSize( 250, 400 ).Dock().CloseButton( false ) );

    m_auimgr.AddPane( m_propsPane, wxAuiPaneInfo().Name( "PropertiesPane" ).Right().Row( 1 )
            .Resizable().MinSize( 250, 400 ).Dock().CloseButton( false ) );

    m_auimgr.Update();

    Raise();
    Show( true );

    Bind( wxEVT_COMMAND_MENU_SELECTED, &LIB_EDIT_FRAME::OnEditSymbolLibTable, this,
          ID_EDIT_SYM_LIB_TABLE );

    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ZOOM_PAGE );
    wxPostEvent( this, evt );
}


LIB_EDIT_FRAME::~LIB_EDIT_FRAME()
{
    Unbind( wxEVT_COMMAND_MENU_SELECTED, &LIB_EDIT_FRAME::OnEditSymbolLibTable, this,
            ID_EDIT_SYM_LIB_TABLE );

    // current screen is destroyed in EDA_DRAW_FRAME
    SetScreen( m_dummyScreen );

    m_lastDrawItem = NULL;
    SetDrawItem( m_lastDrawItem );

    delete m_tempCopyComponent;
    delete m_libMgr;
    delete m_my_part;
}


void LIB_EDIT_FRAME::OnCloseWindow( wxCloseEvent& Event )
{
    if( saveAllLibraries( true ) )
        Destroy();
    else
        Event.Veto();
}


double LIB_EDIT_FRAME::BestZoom()
{
    LIB_PART*  part = GetCurPart();
    double     defaultLibraryZoom = 7.33;

    if( !part )
    {
        SetScrollCenterPosition( wxPoint( 0, 0 ) );
        return defaultLibraryZoom;
    }

    EDA_RECT boundingBox = part->GetUnitBoundingBox( m_unit, m_convert );

    double  sizeX  = (double) boundingBox.GetWidth();
    double  sizeY  = (double) boundingBox.GetHeight();
    wxPoint centre = boundingBox.Centre();

    // Reserve a 20% margin around component bounding box.
    double margin_scale_factor = 1.2;

    return bestZoom( sizeX, sizeY, margin_scale_factor, centre);
}


void LIB_EDIT_FRAME::UpdateAliasSelectList()
{
    if( m_aliasSelectBox == NULL )
        return;

    m_aliasSelectBox->Clear();

    LIB_PART*      part = GetCurPart();

    if( !part )
        return;

    m_aliasSelectBox->Append( part->GetAliasNames() );
    m_aliasSelectBox->SetSelection( 0 );

    int index = m_aliasSelectBox->FindString( m_aliasName );

    if( index != wxNOT_FOUND )
        m_aliasSelectBox->SetSelection( index );
}


void LIB_EDIT_FRAME::UpdatePartSelectList()
{
    if( m_partSelectBox == NULL )
        return;

    if( m_partSelectBox->GetCount() != 0 )
        m_partSelectBox->Clear();

    LIB_PART*      part = GetCurPart();

    if( !part || part->GetUnitCount() <= 1 )
    {
        m_unit = 1;
        m_partSelectBox->Append( wxEmptyString );
    }
    else
    {
        for( int i = 0; i < part->GetUnitCount(); i++ )
        {
            wxString sub  = LIB_PART::SubReference( i+1, false );
            wxString unit = wxString::Format( _( "Unit %s" ), GetChars( sub ) );
            m_partSelectBox->Append( unit );
        }
    }

    // Ensure the current selected unit is compatible with
    // the number of units of the current part:
    if( part && part->GetUnitCount() < m_unit )
        m_unit = 1;

    m_partSelectBox->SetSelection( ( m_unit > 0 ) ? m_unit - 1 : 0 );
}


void LIB_EDIT_FRAME::OnShowElectricalType( wxCommandEvent& event )
{
    SetShowElectricalType( not GetShowElectricalType() );
    GetCanvas()->Refresh();
}


void LIB_EDIT_FRAME::OnToggleSearchTree( wxCommandEvent& event )
{
    auto& treePane = m_auimgr.GetPane( m_treePane );
    treePane.Show( !IsSearchTreeShown() );
    m_auimgr.Update();
}


void LIB_EDIT_FRAME::OnEditSymbolLibTable( wxCommandEvent& aEvent )
{
    SCH_BASE_FRAME::OnEditSymbolLibTable( aEvent );
    SyncLibraries( true );
}


bool LIB_EDIT_FRAME::IsSearchTreeShown()
{
    return m_auimgr.GetPane( m_treePane ).IsShown();
}


void LIB_EDIT_FRAME::ClearSearchTreeSelection()
{
    m_treePane->GetCmpTree()->Unselect();
}


void LIB_EDIT_FRAME::OnUpdateSelectTool( wxUpdateUIEvent& aEvent )
{
    aEvent.Check( GetToolId() == aEvent.GetId() );
}


void LIB_EDIT_FRAME::OnUpdateElectricalType( wxUpdateUIEvent& aEvent )
{
    aEvent.Check( GetShowElectricalType() );
}


void LIB_EDIT_FRAME::OnUpdateEditingPart( wxUpdateUIEvent& aEvent )
{
    LIB_PART* part = GetCurPart();

    aEvent.Enable( part != NULL );

    if( part && aEvent.GetEventObject() == m_drawToolBar )
        aEvent.Check( GetToolId() == aEvent.GetId() );
}


void LIB_EDIT_FRAME::OnUpdatePartModified( wxUpdateUIEvent& aEvent )
{
    LIB_ID libId = getTargetLibId();
    const wxString& partName = libId.GetLibItemName();
    const wxString& libName = libId.GetLibNickname();

    if( aEvent.GetId() == ID_LIBEDIT_SAVE_PART )
    {
        bool readOnly = libName.IsEmpty() || m_libMgr->IsLibraryReadOnly( libName );

        aEvent.SetText( readOnly ? _( "&Save Symbol [Read Only]" ) : _( "&Save Symbol" ) );
        aEvent.Enable( !readOnly && !partName.IsEmpty()
                && m_libMgr->IsPartModified( partName, libName ) );
    }
    else if( aEvent.GetId() == ID_LIBEDIT_REVERT_PART )
    {
        aEvent.Enable( !partName.IsEmpty() && !libName.IsEmpty()
                && m_libMgr->IsPartModified( partName, libName ) );
    }
    else wxFAIL;
}


void LIB_EDIT_FRAME::OnUpdatePaste( wxUpdateUIEvent& event )
{
    event.Enable( m_clipboard.GetCount() > 0 );
}


void LIB_EDIT_FRAME::OnUpdateLibModified( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( m_libMgr->IsLibraryModified( getTargetLib() ) );
}


void LIB_EDIT_FRAME::OnUpdateClipboardNotEmpty( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( !!m_copiedPart );
}


void LIB_EDIT_FRAME::OnUpdateUndo( wxUpdateUIEvent& event )
{
    event.Enable( GetCurPart() && GetScreen() &&
        GetScreen()->GetUndoCommandCount() != 0 && !IsEditingDrawItem() );
}


void LIB_EDIT_FRAME::OnUpdateRedo( wxUpdateUIEvent& event )
{
    event.Enable( GetCurPart() && GetScreen() &&
        GetScreen()->GetRedoCommandCount() != 0 && !IsEditingDrawItem() );
}


void LIB_EDIT_FRAME::OnUpdateSaveLib( wxUpdateUIEvent& event )
{
    wxString lib = getTargetLib();
    bool readOnly = lib.IsEmpty() || m_libMgr->IsLibraryReadOnly( lib );

    event.SetText( readOnly ? _( "&Save Library [Read Only]" ) : _( "&Save Library" ) );
    event.Enable( !readOnly && m_libMgr->IsLibraryModified( lib ) );
}


void LIB_EDIT_FRAME::OnUpdateSaveLibAs( wxUpdateUIEvent& event )
{
    wxString lib = getTargetLib();

    event.Enable( m_libMgr->LibraryExists( lib ) );
}


void LIB_EDIT_FRAME::OnUpdateViewDoc( wxUpdateUIEvent& event )
{
    bool enable = false;

    LIB_PART* part = GetCurPart();

    if( part )
    {
        LIB_ALIAS* alias = part->GetAlias( m_aliasName );

        wxCHECK_RET( alias != NULL,
                     wxString::Format( "Alias \"%s\" not found in symbol \"%s\".",
                                       m_aliasName, part->GetName() ) );

        enable = !alias->GetDocFileName().IsEmpty();
    }

    event.Enable( enable );
}


void LIB_EDIT_FRAME::OnUpdateSyncPinEdit( wxUpdateUIEvent& event )
{
    LIB_PART* part = GetCurPart();
    event.Enable( part && part->IsMulti() && !part->UnitsLocked() );
    event.Check( m_syncPinEdit );
}


void LIB_EDIT_FRAME::OnUpdatePinTable( wxUpdateUIEvent& event )
{
    LIB_PART* part = GetCurPart();
    event.Enable( part != NULL );
}


void LIB_EDIT_FRAME::OnUpdatePartNumber( wxUpdateUIEvent& event )
{
    if( m_partSelectBox == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    // Using the typical event.Enable() call doesn't seem to work with wxGTK
    // so use the pointer to alias combobox to directly enable or disable.
    m_partSelectBox->Enable( part && part->GetUnitCount() > 1 );
}


void LIB_EDIT_FRAME::OnUpdateDeMorganNormal( wxUpdateUIEvent& event )
{
    if( m_mainToolBar == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    event.Enable( GetShowDeMorgan() || ( part && part->HasConversion() ) );
    event.Check( m_convert <= 1 );
}


void LIB_EDIT_FRAME::OnUpdateDeMorganConvert( wxUpdateUIEvent& event )
{
    if( m_mainToolBar == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    event.Enable( GetShowDeMorgan() || ( part && part->HasConversion() ) );
    event.Check( m_convert > 1 );
}


void LIB_EDIT_FRAME::OnUpdateSelectAlias( wxUpdateUIEvent& event )
{
    if( m_aliasSelectBox == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    // Using the typical event.Enable() call doesn't seem to work with wxGTK
    // so use the pointer to alias combobox to directly enable or disable.
    m_aliasSelectBox->Enable( part && part->GetAliasCount() > 1 );
}


void LIB_EDIT_FRAME::OnSelectAlias( wxCommandEvent& event )
{
    if( m_aliasSelectBox == NULL
        || ( m_aliasSelectBox->GetStringSelection().CmpNoCase( m_aliasName ) == 0)  )
        return;

    m_lastDrawItem = NULL;
    m_aliasName = m_aliasSelectBox->GetStringSelection();

    DisplayCmpDoc();
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::OnSelectPart( wxCommandEvent& event )
{
    int i = event.GetSelection();

    if( ( i == wxNOT_FOUND ) || ( ( i + 1 ) == m_unit ) )
        return;

    m_lastDrawItem = NULL;
    m_unit = i + 1;
    m_canvas->Refresh();
    DisplayCmpDoc();
}


void LIB_EDIT_FRAME::OnViewEntryDoc( wxCommandEvent& event )
{
    LIB_PART* part = GetCurPart();

    if( !part )
        return;

    wxString    fileName;
    LIB_ALIAS*  alias = part->GetAlias( m_aliasName );

    wxCHECK_RET( alias != NULL, "Alias not found." );

    fileName = alias->GetDocFileName();

    if( !fileName.IsEmpty() )
    {
        SEARCH_STACK* lib_search = Prj().SchSearchS();

        GetAssociatedDocument( this, fileName, lib_search );
    }
}


void LIB_EDIT_FRAME::OnSelectBodyStyle( wxCommandEvent& event )
{
    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    if( event.GetId() == ID_DE_MORGAN_NORMAL_BUTT )
        m_convert = 1;
    else
        m_convert = 2;

    m_lastDrawItem = NULL;
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::Process_Special_Functions( wxCommandEvent& event )
{
    int     id = event.GetId();
    wxPoint pos;
    SCH_SCREEN* screen = GetScreen();
    BLOCK_SELECTOR& block = screen->m_BlockLocate;
    LIB_ITEM* item = screen->GetCurLibItem();

    m_canvas->SetIgnoreMouseEvents( true );

    wxGetMousePosition( &pos.x, &pos.y );
    pos.y += 20;

    switch( id )   // Stop placement commands before handling new command.
    {
    case wxID_COPY:
    case wxID_CUT:
    case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
    case ID_LIBEDIT_EDIT_PIN:
    case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
    case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
    case ID_POPUP_ZOOM_BLOCK:
    case ID_POPUP_DELETE_BLOCK:
    case ID_POPUP_DUPLICATE_BLOCK:
    case ID_POPUP_SELECT_ITEMS_BLOCK:
    case ID_POPUP_MIRROR_X_BLOCK:
    case ID_POPUP_MIRROR_Y_BLOCK:
    case ID_POPUP_ROTATE_BLOCK:
    case ID_POPUP_PLACE_BLOCK:
    case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
        break;

    case ID_POPUP_LIBEDIT_CANCEL_EDITING:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->EndMouseCapture();
        else
            m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );
        break;

    case ID_POPUP_LIBEDIT_DELETE_ITEM:
        m_canvas->EndMouseCapture();
        break;

    default:
        m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(),
                                   wxEmptyString );
        break;
    }

    INSTALL_UNBUFFERED_DC( dc, m_canvas );

    switch( id )
    {
    case ID_POPUP_LIBEDIT_CANCEL_EDITING:
        break;

    case ID_LIBEDIT_SYNC_PIN_EDIT:
        m_syncPinEdit = m_mainToolBar->GetToolToggled( ID_LIBEDIT_SYNC_PIN_EDIT );
        break;

    case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
        m_canvas->MoveCursorToCrossHair();
        if( item )
        {
            EndDrawGraphicItem( &dc );
        }
        break;

    case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
        if( item )
        {
            m_canvas->CrossHairOff( &dc );

            switch( item->Type() )
            {
            case LIB_ARC_T:
            case LIB_CIRCLE_T:
            case LIB_RECTANGLE_T:
            case LIB_POLYLINE_T:
                EditGraphicSymbol( &dc, item );
                break;

            case LIB_TEXT_T:
                EditSymbolText( &dc, item );
                break;

            default:
                ;
            }

            m_canvas->CrossHairOn( &dc );
        }
        break;

    case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
        {
            // Delete the last created segment, while creating a polyline draw item
            if( item == NULL )
                break;

            m_canvas->MoveCursorToCrossHair();
            STATUS_FLAGS oldFlags = item->GetFlags();
            item->ClearFlags();
            item->Draw( m_canvas, &dc, wxPoint( 0, 0 ), COLOR4D::UNSPECIFIED, g_XorMode, NULL,
                              DefaultTransform );
            ( (LIB_POLYLINE*) item )->DeleteSegment( GetCrossHairPosition( true ) );
            item->Draw( m_canvas, &dc, wxPoint( 0, 0 ), COLOR4D::UNSPECIFIED, g_XorMode, NULL,
                              DefaultTransform );
            item->SetFlags( oldFlags );
            m_lastDrawItem = NULL;
        }
        break;

    case ID_POPUP_LIBEDIT_DELETE_ITEM:
        if( item )
            deleteItem( &dc );

        break;

    case ID_POPUP_LIBEDIT_MOVE_ITEM_REQUEST:
        if( item == NULL )
            break;

        if( item->Type() == LIB_PIN_T )
            StartMovePin( &dc );
        else
            StartMoveDrawSymbol( &dc );
        break;

    case ID_POPUP_LIBEDIT_MODIFY_ITEM:

        if( item == NULL )
            break;

        m_canvas->MoveCursorToCrossHair();
        if( item->Type() == LIB_RECTANGLE_T
            || item->Type() == LIB_CIRCLE_T
            || item->Type() == LIB_POLYLINE_T
            || item->Type() == LIB_ARC_T
            )
        {
            StartModifyDrawSymbol( &dc );
        }

        break;

    case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
        if( item == NULL )
            break;

        m_canvas->CrossHairOff( &dc );

        if( item->Type() == LIB_FIELD_T )
        {
            EditField( (LIB_FIELD*) item );
        }

        m_canvas->MoveCursorToCrossHair();
        m_canvas->CrossHairOn( &dc );
        break;

    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
        {
            if( !item || item->Type() != LIB_PIN_T )
                break;

            LIB_PART*      part = GetCurPart();

            SaveCopyInUndoList( part );

            GlobalSetPins( (LIB_PIN*) item, id );
            m_canvas->MoveCursorToCrossHair();
            m_canvas->Refresh();
        }
        break;

    case ID_POPUP_ZOOM_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_ZOOM );
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_DELETE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_DELETE );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_DUPLICATE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_DUPLICATE );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_SELECT_ITEMS_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_SELECT_ITEMS_ONLY );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_MIRROR_Y_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_MIRROR_Y );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_MIRROR_X_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_MIRROR_X );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_ROTATE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        block.SetCommand( BLOCK_ROTATE );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_PLACE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case wxID_COPY:
        block.SetCommand( BLOCK_COPY );
        block.SetMessageBlock( this );
        HandleBlockEnd( &dc );
        break;

    case wxID_PASTE:
        HandleBlockBegin( &dc, BLOCK_PASTE, GetCrossHairPosition() );
        break;

    case wxID_CUT:
        if( block.GetCommand() != BLOCK_MOVE )
            break;

        block.SetCommand( BLOCK_CUT );
        block.SetMessageBlock( this );
        HandleBlockEnd( &dc );
        break;

    default:
        DisplayError( this, "LIB_EDIT_FRAME::Process_Special_Functions error" );
        break;
    }

    m_canvas->SetIgnoreMouseEvents( false );

    if( GetToolId() == ID_NO_TOOL_SELECTED )
        m_lastDrawItem = NULL;
}


void LIB_EDIT_FRAME::OnActivate( wxActivateEvent& event )
{
    EDA_DRAW_FRAME::OnActivate( event );
}


wxString LIB_EDIT_FRAME::GetCurLib() const
{
    wxString libNickname = Prj().GetRString( PROJECT::SCH_LIBEDIT_CUR_LIB );

    if( !libNickname.empty() )
    {
        if( !Prj().SchSymbolLibTable()->HasLibrary( libNickname ) )
        {
            Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, wxEmptyString );
            libNickname = wxEmptyString;
        }
    }

    return libNickname;
}


wxString LIB_EDIT_FRAME::SetCurLib( const wxString& aLibNickname )
{
    wxString old = GetCurLib();

    if( aLibNickname.empty() || !Prj().SchSymbolLibTable()->HasLibrary( aLibNickname ) )
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, wxEmptyString );
    else
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, aLibNickname );

    m_libMgr->SetCurrentLib( aLibNickname );

    return old;
}


void LIB_EDIT_FRAME::SetCurPart( LIB_PART* aPart )
{
    wxASSERT( m_my_part != aPart );

    if( m_my_part != aPart )
    {
        delete m_my_part;
        m_my_part = aPart;
    }

    // select the current component in the tree widget
    if( aPart )
        m_treePane->GetCmpTree()->SelectLibId( aPart->GetLibId() );

    wxString partName = aPart ? aPart->GetName() : wxString();
    m_libMgr->SetCurrentPart( partName );

    // retain in case this wxFrame is re-opened later on the same PROJECT
    Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_PART, partName );

    // Ensure synchronized pin edit can be enabled only symbols with interchangeable units
    m_syncPinEdit = aPart && aPart->IsMulti() && !aPart->UnitsLocked();

	// load the part properties into the properties dialog
	m_propsPane->SetPart(aPart);
}


void LIB_EDIT_FRAME::TempCopyComponent()
{
    delete m_tempCopyComponent;

    if( LIB_PART* part = GetCurPart() )
        // clone it and own the clone.
        m_tempCopyComponent = new LIB_PART( *part );
    else
        // clear it, there was no CurPart
        m_tempCopyComponent = NULL;
}


void LIB_EDIT_FRAME::RestoreComponent()
{
    if( m_tempCopyComponent )
    {
        // transfer ownership to CurPart
        SetCurPart( m_tempCopyComponent );
        m_tempCopyComponent = NULL;
    }
}


void LIB_EDIT_FRAME::ClearTempCopyComponent()
{
    delete m_tempCopyComponent;
    m_tempCopyComponent = NULL;
}


void LIB_EDIT_FRAME::EditSymbolText( wxDC* DC, LIB_ITEM* DrawItem )
{
    if ( ( DrawItem == NULL ) || ( DrawItem->Type() != LIB_TEXT_T ) )
        return;

    // Deleting old text
    if( DC && !DrawItem->InEditMode() )
        DrawItem->Draw( m_canvas, DC, wxPoint( 0, 0 ), COLOR4D::UNSPECIFIED, g_XorMode, NULL,
                        DefaultTransform );

    DIALOG_LIB_EDIT_TEXT* frame = new DIALOG_LIB_EDIT_TEXT( this, (LIB_TEXT*) DrawItem );
    frame->ShowModal();
    frame->Destroy();
    OnModify();

    // Display new text
    if( DC && !DrawItem->InEditMode() )
        DrawItem->Draw( m_canvas, DC, wxPoint( 0, 0 ), COLOR4D::UNSPECIFIED, GR_DEFAULT_DRAWMODE,
                        NULL, DefaultTransform );
}


void LIB_EDIT_FRAME::OnEditComponentProperties( wxCommandEvent& event )
{
    bool partLocked = GetCurPart()->UnitsLocked();
    wxString oldName = GetCurPart()->GetName();

    DIALOG_EDIT_COMPONENT_IN_LIBRARY dlg( this );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    // if m_UnitSelectionLocked has changed, set some edit options or defaults
    // to the best value
    if( partLocked != GetCurPart()->UnitsLocked() )
    {
        // Enable synchronized pin edit mode for symbols with interchangeable units
        m_syncPinEdit = !GetCurPart()->UnitsLocked();
        // also set default edit options to the better value
        // Usually if units are locked, graphic items are specific to each unit
        // and if units are interchangeable, graphic items are common to units
        m_drawSpecificUnit = GetCurPart()->UnitsLocked() ? true : false;
    }

    if( oldName != GetCurPart()->GetName() )
        m_libMgr->RemovePart( GetCurLib(), oldName );

    m_libMgr->UpdatePart( GetCurPart(), GetCurLib() );

    UpdateAliasSelectList();
    UpdatePartSelectList();
    DisplayLibInfos();
    DisplayCmpDoc();
    OnModify();
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::OnSelectTool( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();
    int lastToolID = GetToolId();

    if( GetToolId() == ID_NO_TOOL_SELECTED || GetToolId() == ID_ZOOM_SELECTION )
        m_lastDrawItem = NULL;

    // Stop the current command and deselect the current tool.
    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    LIB_PART*      part = GetCurPart();

    switch( id )
    {
    case ID_NO_TOOL_SELECTED:
        SetToolID( id, m_canvas->GetDefaultCursor(), wxEmptyString );
        break;

    case ID_ZOOM_SELECTION:
        // This tool is located on the main toolbar: switch it on or off on click on it
        if( lastToolID != ID_ZOOM_SELECTION )
            SetToolID( ID_ZOOM_SELECTION, wxCURSOR_MAGNIFIER, _( "Zoom to selection" ) );
        else
            SetNoToolSelected();
        break;

    case ID_LIBEDIT_PIN_BUTT:
        if( part )
        {
            SetToolID( id, wxCURSOR_PENCIL, _( "Add pin" ) );
        }
        else
        {
            SetToolID( id, wxCURSOR_ARROW, _( "Set pin options" ) );

            wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );

            cmd.SetId( ID_LIBEDIT_EDIT_PIN );
            GetEventHandler()->ProcessEvent( cmd );
            SetNoToolSelected();
        }
        break;

    case ID_LIBEDIT_BODY_TEXT_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add text" ) );
        break;

    case ID_LIBEDIT_BODY_RECT_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add rectangle" ) );
        break;

    case ID_LIBEDIT_BODY_CIRCLE_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add circle" ) );
        break;

    case ID_LIBEDIT_BODY_ARC_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add arc" ) );
        break;

    case ID_LIBEDIT_BODY_LINE_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add line" ) );
        break;

    case ID_LIBEDIT_ANCHOR_ITEM_BUTT:
        SetToolID( id, wxCURSOR_HAND, _( "Set anchor position" ) );
        break;

    case ID_LIBEDIT_IMPORT_BODY_BUTT:
        SetToolID( id, m_canvas->GetDefaultCursor(), _( "Import" ) );
        LoadOneSymbol();
        SetNoToolSelected();
        break;

    case ID_LIBEDIT_EXPORT_BODY_BUTT:
        SetToolID( id, m_canvas->GetDefaultCursor(), _( "Export" ) );
        SaveOneSymbol();
        SetNoToolSelected();
        break;

    case ID_LIBEDIT_DELETE_ITEM_BUTT:
        if( !part )
        {
            wxBell();
            break;
        }

        SetToolID( id, wxCURSOR_BULLSEYE, _( "Delete item" ) );
        break;

    default:
        break;
    }

    m_canvas->SetIgnoreMouseEvents( false );
}


void LIB_EDIT_FRAME::OnRotateItem( wxCommandEvent& aEvent )
{
    LIB_ITEM* item = GetDrawItem();

    if( item == NULL )
        return;

    if( !item->InEditMode() )
    {
        LIB_PART*      part = GetCurPart();

        SaveCopyInUndoList( part );
        item->SetUnit( m_unit );
    }

    item->Rotate();
    OnModify();

    if( !item->InEditMode() )
        item->ClearFlags();

    m_canvas->Refresh();

    if( GetToolId() == ID_NO_TOOL_SELECTED )
        m_lastDrawItem = NULL;
}


void LIB_EDIT_FRAME::OnOrient( wxCommandEvent& aEvent )
{
    INSTALL_UNBUFFERED_DC( dc, m_canvas );
    SCH_SCREEN* screen = GetScreen();
    // Allows block rotate operation on hot key.
    if( screen->m_BlockLocate.GetState() != STATE_NO_BLOCK )
    {
        if( aEvent.GetId() == ID_LIBEDIT_MIRROR_X )
        {
            m_canvas->MoveCursorToCrossHair();
            screen->m_BlockLocate.SetMessageBlock( this );
            screen->m_BlockLocate.SetCommand( BLOCK_MIRROR_X );
            HandleBlockEnd( &dc );
        }
        else if( aEvent.GetId() == ID_LIBEDIT_MIRROR_Y )
        {
            m_canvas->MoveCursorToCrossHair();
            screen->m_BlockLocate.SetMessageBlock( this );
            screen->m_BlockLocate.SetCommand( BLOCK_MIRROR_Y );
            HandleBlockEnd( &dc );
        }
    }
}


LIB_ITEM* LIB_EDIT_FRAME::LocateItemUsingCursor( const wxPoint& aPosition,
                                                 const KICAD_T aFilterList[] )
{
    wxPoint        pos;
    LIB_PART*      part = GetCurPart();

    if( !part )
        return NULL;

    LIB_ITEM* item = locateItem( aPosition, aFilterList );

    // If the user aborted the clarification context menu, don't show it again at the
    // grid position.
    if( !item && m_canvas->GetAbortRequest() )
        return NULL;

    pos = GetNearestGridPosition( aPosition );

    if( item == NULL && aPosition != pos )
        item = locateItem( pos, aFilterList );

    return item;
}


LIB_ITEM* LIB_EDIT_FRAME::locateItem( const wxPoint& aPosition, const KICAD_T aFilterList[] )
{
    LIB_PART*      part = GetCurPart();

    if( !part )
        return NULL;

    LIB_ITEM* item = NULL;

    m_collectedItems.Collect( part->GetDrawItems(), aFilterList, aPosition,
                              m_unit, m_convert );

    if( m_collectedItems.GetCount() == 0 )
    {
        ClearMsgPanel();
    }
    else if( m_collectedItems.GetCount() == 1 )
    {
        item = m_collectedItems[0];
    }
    else
    {
        if( item == NULL )
        {
            wxASSERT_MSG( m_collectedItems.GetCount() <= MAX_SELECT_ITEM_IDS,
                          "Select item clarification context menu size limit exceeded." );

            wxMenu selectMenu;
            AddMenuItem( &selectMenu, wxID_NONE, _( "Clarify Selection" ),
                         KiBitmap( info_xpm ) );

            selectMenu.AppendSeparator();

            for( int i = 0;  i < m_collectedItems.GetCount() && i < MAX_SELECT_ITEM_IDS;  i++ )
            {
                wxString    text = m_collectedItems[i]->GetSelectMenuText();
                BITMAP_DEF  xpm = m_collectedItems[i]->GetMenuImage();

                AddMenuItem( &selectMenu, ID_SELECT_ITEM_START + i, text, KiBitmap( xpm ) );
            }

            // Set to NULL in case user aborts the clarification context menu.
            SetDrawItem( NULL );
            m_canvas->SetAbortRequest( true );   // Changed to false if an item is selected
            PopupMenu( &selectMenu );
            m_canvas->MoveCursorToCrossHair();
            item = GetDrawItem();
        }
    }

    if( item )
    {
        MSG_PANEL_ITEMS items;
        item->GetMsgPanelInfo( items );
        SetMsgPanel( items );
    }
    else
    {
        ClearMsgPanel();
    }

    return item;
}


void LIB_EDIT_FRAME::deleteItem( wxDC* aDC )
{
    LIB_ITEM* item = GetDrawItem();

    wxCHECK_RET( item != NULL, "No drawing item selected to delete." );

    m_canvas->CrossHairOff( aDC );

    LIB_PART*      part = GetCurPart();

    SaveCopyInUndoList( part );

    if( item->Type() == LIB_PIN_T )
    {
        LIB_PIN*    pin = static_cast<LIB_PIN*>( item );
        wxPoint     pos = pin->GetPosition();

        part->RemoveDrawItem( (LIB_ITEM*) pin, m_canvas, aDC );

        // when pin edition is synchronized, all pins of the same body style
        // are removed:
        if( SynchronizePins() )
        {
            int curr_convert = pin->GetConvert();
            LIB_PIN* next_pin = part->GetNextPin();

            while( next_pin != NULL )
            {
                pin = next_pin;
                next_pin = part->GetNextPin( pin );

                if( pin->GetPosition() != pos )
                    continue;

                if( pin->GetConvert() != curr_convert )
                    continue;

                part->RemoveDrawItem( pin );
            }
        }

        m_canvas->Refresh();
    }
    else
    {
        if( m_canvas->IsMouseCaptured() )
        {
            m_canvas->CallEndMouseCapture( aDC );
        }
        else
        {
            part->RemoveDrawItem( item, m_canvas, aDC );
            m_canvas->Refresh();
        }
    }

    SetDrawItem( NULL );
    m_lastDrawItem = NULL;
    OnModify();
    m_canvas->CrossHairOn( aDC );
}


void LIB_EDIT_FRAME::OnModify()
{
    GetScreen()->SetModify();
    storeCurrentPart();
    m_treePane->GetCmpTree()->Refresh();
}


void LIB_EDIT_FRAME::OnSelectItem( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();
    int index = id - ID_SELECT_ITEM_START;

    if( (id >= ID_SELECT_ITEM_START && id <= ID_SELECT_ITEM_END)
        && (index >= 0 && index < m_collectedItems.GetCount()) )
    {
        LIB_ITEM* item = m_collectedItems[index];
        m_canvas->SetAbortRequest( false );
        SetDrawItem( item );
    }
}


void LIB_EDIT_FRAME::OnOpenPinTable( wxCommandEvent& aEvent )
{
    LIB_PART* part = GetCurPart();

    DIALOG_LIB_EDIT_PIN_TABLE dlg( this, *part );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    return;
}


bool LIB_EDIT_FRAME::SynchronizePins()
{
    LIB_PART* part = GetCurPart();

    return m_syncPinEdit && part && part->IsMulti() && !part->UnitsLocked();
}


void LIB_EDIT_FRAME::refreshSchematic()
{
    // This is not the most effecient way to do this because the changed library may not have
    // any effect on the schematic symbol links.  Since this is not called very often, take the
    // hit here rather than the myriad other places (including SCH_SCREEN::Draw()) where it was
    // being called.
    SCH_SCREENS schematic;

    schematic.UpdateSymbolLinks();
    schematic.TestDanglingEnds();

    // There may be no parent window so use KIWAY message to refresh the schematic editor
    // in case any symbols have changed.
    Kiway().ExpressMail( FRAME_SCH, MAIL_SCH_REFRESH, std::string( "" ), this );
}


bool LIB_EDIT_FRAME::addLibraryFile( bool aCreateNew )
{
    wxFileName fileName = getLibraryFileName( !aCreateNew );
    wxString libName = fileName.GetName();
    bool res = false;

    if( libName.IsEmpty() )
        return false;

    if( m_libMgr->LibraryExists( libName ) )
    {
        DisplayError( this,
                wxString::Format( _( "Library \"%s\" already exists" ), GetChars( libName ) ) );
        return false;
    }

    // Select the target library table (global/project)
    SYMBOL_LIB_TABLE* libTable = selectSymLibTable();

    if( !libTable )
        return false;

    if( aCreateNew )
    {
        res = m_libMgr->CreateLibrary( fileName.GetFullPath(), libTable );

        if( !res )
            DisplayError( this, _( "Could not create the library file. Check write permission." ) );
    }
    else
    {
        res = m_libMgr->AddLibrary( fileName.GetFullPath(), libTable );

        if( !res )
            DisplayError( this, _( "Could not open the library file." ) );
    }

    bool globalTable = ( libTable == &SYMBOL_LIB_TABLE::GetGlobalLibTable() );
    saveSymbolLibTables( globalTable, !globalTable );

    return res;
}


wxFileName LIB_EDIT_FRAME::getLibraryFileName( bool aExisting )
{
    wxFileName fn = m_libMgr->GetUniqueLibraryName();
    fn.SetExt( SchematicLibraryFileExtension );

    wxFileDialog dlg( this,
            aExisting ? _( "Select Library" ) : _( "New Library" ),
            Prj().GetProjectPath(),
            aExisting ? wxString( wxEmptyString ) : fn.GetFullName() ,
            SchematicLibraryFileWildcard(),
            aExisting ? wxFD_OPEN | wxFD_FILE_MUST_EXIST :
                         wxFD_SAVE | wxFD_CHANGE_DIR | wxFD_OVERWRITE_PROMPT );

    if( dlg.ShowModal() == wxID_CANCEL )
        return wxFileName();

    fn = dlg.GetPath();
    fn.SetExt( SchematicLibraryFileExtension );

    return fn;
}


LIB_PART* LIB_EDIT_FRAME::getTargetPart() const
{
    LIB_ALIAS* alias = nullptr;

    if( m_treePane->GetCmpTree()->IsMenuActive() )
    {
        LIB_ID libId = m_treePane->GetCmpTree()->GetSelectedLibId();
        alias = m_libMgr->GetAlias( libId.GetLibItemName(), libId.GetLibNickname() );
    }
    else if( LIB_PART* part = GetCurPart() )
    {
        alias = part->GetAlias( 0 );
    }

    return alias ? alias->GetPart() : nullptr;
}


LIB_ID LIB_EDIT_FRAME::getTargetLibId() const
{
    if( m_treePane->GetCmpTree()->IsMenuActive() )
        return m_treePane->GetCmpTree()->GetSelectedLibId();

    if( LIB_PART* part = GetCurPart() )
        return part->GetLibId();

    return LIB_ID();
}


wxString LIB_EDIT_FRAME::getTargetLib() const
{
    if( m_treePane->GetCmpTree()->IsMenuActive() )
    {
        LIB_ID libId = m_treePane->GetCmpTree()->GetSelectedLibId();
        return libId.GetLibNickname();
    }
    else
    {
        return GetCurLib();
    }
}


void LIB_EDIT_FRAME::SyncLibraries( bool aProgress )
{
    LIB_ID selected;

    if( m_treePane )
        selected = m_treePane->GetCmpTree()->GetSelectedLibId();

    if( aProgress )
    {
        wxProgressDialog progressDlg( _( "Loading Symbol Libraries" ),
                wxEmptyString, m_libMgr->GetAdapter()->GetLibrariesCount(), this );

        m_libMgr->Sync( true, [&]( int progress, int max, const wxString& libName ) {
            progressDlg.Update( progress, wxString::Format( _( "Loading library \"%s\"" ), libName ) );
        } );
    }
    else
    {
        m_libMgr->Sync( true );
    }

    if( m_treePane )
    {
        wxDataViewItem found;

        if( selected.IsValid() )
        {
            // Check if the previously selected item is still valid,
            // if not - it has to be unselected to prevent crash
            found = m_libMgr->GetAdapter()->FindItem( selected );

            if( !found )
                m_treePane->GetCmpTree()->Unselect();
        }

        m_treePane->Regenerate();

        // Try to select the parent library, in case the part is not found
        if( !found && selected.IsValid() )
        {
            selected.SetLibItemName( "" );
            found = m_libMgr->GetAdapter()->FindItem( selected );

            if( found )
                m_treePane->GetCmpTree()->SelectLibId( selected );
        }
    }
}


SYMBOL_LIB_TABLE* LIB_EDIT_FRAME::selectSymLibTable()
{
    wxArrayString libTableNames;
    libTableNames.Add( _( "Global" ) );
    libTableNames.Add( _( "Project" ) );

    switch( SelectSingleOption( this, _( "Select Symbol Library Table" ),
                _( "Choose the Library Table to add the library:" ), libTableNames ) )
    {
        case 0: return &SYMBOL_LIB_TABLE::GetGlobalLibTable();
        case 1: return Prj().SchSymbolLibTable();
    }

    return nullptr;
}


bool LIB_EDIT_FRAME::backupFile( const wxFileName& aOriginalFile, const wxString& aBackupExt )
{
    if( aOriginalFile.FileExists() )
    {
        wxFileName backupFileName( aOriginalFile );
        backupFileName.SetExt( "bck" );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxCopyFile( aOriginalFile.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            DisplayError( this, _( "Failed to save backup document to file " ) +
                  backupFileName.GetFullPath() );
            return false;
        }
    }

    return true;
}


void LIB_EDIT_FRAME::storeCurrentPart()
{
    if( m_my_part && !GetCurLib().IsEmpty() && GetScreen()->IsModify() )
        m_libMgr->UpdatePart( m_my_part, GetCurLib() ); // UpdatePart() makes a copy
}


bool LIB_EDIT_FRAME::isCurrentPart( const LIB_ID& aLibId ) const
{
    // This will return the root part of any alias
    LIB_PART* part = m_libMgr->GetBufferedPart( aLibId.GetLibItemName(), aLibId.GetLibNickname() );
    // Now we can compare the libId of the current part and the root part
    return ( GetCurPart() && part->GetLibId() == GetCurPart()->GetLibId() );
}


void LIB_EDIT_FRAME::emptyScreen()
{
    SetCurLib( wxEmptyString );
    SetCurPart( nullptr );
    m_aliasName.Empty();
    m_lastDrawItem = nullptr;
    SetDrawItem( NULL );
    SetScreen( m_dummyScreen );
    m_dummyScreen->ClearUndoRedoList();
    Zoom_Automatique( false );
    Refresh();
}


int LIB_EDIT_FRAME::GetIconScale()
{
    int scale = 0;
    Kiface().KifaceSettings()->Read( LibIconScaleEntry, &scale, 0 );
    return scale;
}


void LIB_EDIT_FRAME::SetIconScale( int aScale )
{
    Kiface().KifaceSettings()->Write( LibIconScaleEntry, aScale );
    ReCreateMenuBar();
    ReCreateVToolbar();
    ReCreateHToolbar();
    ReCreateOptToolbar();
    Layout();
    SendSizeEvent();
}

void LIB_EDIT_FRAME::OnUpdateFieldValue(LIB_FIELD *aField, const wxString &newFieldValue){
    LIB_PART* parent = aField->GetParent();
    wxString oldFieldValue = aField->GetFullText( m_unit );
    bool renamed = aField->GetId() == VALUE && newFieldValue != oldFieldValue;

    if( renamed )
    {
        wxString msg;
        wxString lib = GetCurLib();

        // Test the current library for name conflicts
        if( !lib.empty() && m_libMgr->PartExists( newFieldValue, lib ) )
        {
            msg.Printf( _(
                "The name \"%s\" conflicts with an existing entry in the symbol library \"%s\"." ),
                newFieldValue, lib );

            DisplayErrorMessage( this, msg );
            return;
        }

        SaveCopyInUndoList( parent, UR_LIB_RENAME );
        parent->SetName( newFieldValue );

        if( !parent->HasAlias( m_aliasName ) )
            m_aliasName = newFieldValue;

        m_libMgr->UpdatePartAfterRename( parent, oldFieldValue, lib );

        // Reselect the renamed part
        m_treePane->GetCmpTree()->SelectLibId( LIB_ID( lib, newFieldValue ) );
    }

    aField->SetText(newFieldValue);

    if( !aField->InEditMode() && !renamed )
        SaveCopyInUndoList( parent );

    m_canvas->Refresh();

    OnModify();
    UpdateAliasSelectList();
}

