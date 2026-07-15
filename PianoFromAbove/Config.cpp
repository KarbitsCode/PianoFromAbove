/*************************************************************************************************
*
* File: Config.cpp
*
* Description: Implements the configuration objects
*
* Copyright (c) 2010 Brian Pantano. All rights reserved.
*
*************************************************************************************************/
#include <Windows.h>
#include <Shlobj.h>
#include <TChar.h>

#include <fstream>
#include <thread>
#include <atomic>
using namespace std;

#include "Config.h"
#include "Misc.h"
//-----------------------------------------------------------------------------
// Main Config class
//-----------------------------------------------------------------------------

Config &Config::GetConfig()
{
    static Config instance;
    return instance;
}

Config::Config()
{
    LoadDefaultValues();
    LoadConfigValues();

    ProgressScanDialog progressDlg;
    if ( progressDlg.Create() )
    {
        int iTotalFiles = m_SongLibrary.CountFilesInSource();
        m_SongLibrary.SetProgressCallback([&progressDlg, iTotalFiles]( int iFilesScanned, const wstring& sCurrentFile ) {
            progressDlg.SetProgress( iFilesScanned, iTotalFiles, sCurrentFile );
        });

        // Scan on background thread
        atomic< bool > bScanComplete( false );
        thread scanThread([this, &bScanComplete]() {
            m_SongLibrary.ExpandSources();
            bScanComplete = true;
        });

        while ( !bScanComplete && progressDlg.IsValid() )
            progressDlg.ProcessMessages();

        scanThread.join();
    }
}

string Config::GetFolder()
{
    char sAppData[MAX_PATH];
    if ( FAILED( SHGetFolderPathA( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, sAppData ) ) )
        return string();

    strcat_s( sAppData, "\\" );
    strcat_s( sAppData, APPNAME );
    if ( GetFileAttributesA( sAppData ) == INVALID_FILE_ATTRIBUTES )
        if ( !CreateDirectoryA( sAppData, NULL ) )
            return string();

    return sAppData;
}

void Config::LoadDefaultValues()
{
    m_VisualSettings.LoadDefaultValues();
    m_AudioSettings.LoadDefaultValues();
    m_VideoSettings.LoadDefaultValues();
    m_ControlsSettings.LoadDefaultValues();
    m_SongLibrary.LoadDefaultValues();
    m_PlaybackSettings.LoadDefaultValues();
    m_ViewSettings.LoadDefaultValues();
}

void Config::LoadConfigValues()
{
    // Where to load?
    string sPath = GetFolder();
    if ( sPath.length() == 0 ) return;

    // Load it
    TiXmlDocument doc( sPath + "\\Config.xml" );
    if ( !doc.LoadFile() ) return;

    // Get the root element
    TiXmlElement *txRoot = doc.FirstChildElement();
    if ( !txRoot ) return;

    LoadConfigValues( txRoot );
}

void Config::LoadConfigValues( TiXmlElement *txRoot )
{
    m_VisualSettings.LoadConfigValues( txRoot );
    m_AudioSettings.LoadConfigValues( txRoot );
    m_VideoSettings.LoadConfigValues( txRoot );
    m_ControlsSettings.LoadConfigValues( txRoot );
    m_SongLibrary.LoadConfigValues( txRoot );
    m_PlaybackSettings.LoadConfigValues( txRoot );
    m_ViewSettings.LoadConfigValues( txRoot );
}

bool Config::SaveConfigValues()
{
    // Where to save?
    string sPath = GetFolder();
    if ( sPath.length() == 0 ) return false;

    // Create the XML document
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );
    TiXmlElement *txRoot = new TiXmlElement( APPNAMENOSPACES );
    doc.LinkEndChild( txRoot );

    // Save each of the config
    SaveConfigValues( txRoot );

    // Write it!
    return doc.SaveFile( sPath + "\\Config.xml" );
}

bool Config::SaveConfigValues( TiXmlElement *txRoot )
{
    bool bSaved = true;
    bSaved &= m_VisualSettings.SaveConfigValues( txRoot );
    bSaved &= m_AudioSettings.SaveConfigValues( txRoot );
    bSaved &= m_VideoSettings.SaveConfigValues( txRoot );
    bSaved &= m_ControlsSettings.SaveConfigValues( txRoot );
    bSaved &= m_SongLibrary.SaveConfigValues( txRoot );
    bSaved &= m_PlaybackSettings.SaveConfigValues( txRoot );
    bSaved &= m_ViewSettings.SaveConfigValues( txRoot );
    return bSaved;
}

//-----------------------------------------------------------------------------
// LoadDefaultValues
//-----------------------------------------------------------------------------

void VisualSettings::LoadDefaultValues()
{
    this->eKeysShown = All;
    this->eRenderMode = Time;
    this->eAccidentals = Sharps;
    this->bAlwaysShowControls = false;
    this->bAssociateFiles = false;
    this->iFirstKey = MIDI::A0;
    this->iLastKey = MIDI::C8;

    iBkgColor = 0x00303030;
    int R, G, B, H = 0, S = 80, V = 100;
    int iColors = sizeof( this->colors ) / sizeof( this->colors[0] );
    for ( int i = 10, count = 0; count < iColors; i = ( i + 7 ) % iColors, count++ )
    {
        Util::HSVtoRGB( 360 * i / iColors, S, V, R, G, B );
        this->colors[count] = RGB( R, G, B );
    }
    swap( this->colors[2], this->colors[4] );
}

void AudioSettings::LoadDefaultValues()
{
    this->iOutDevice = -1;
    LoadMIDIDevices();
}

void VideoSettings::LoadDefaultValues()
{
    this->bOpaqueStatus = false;
    this->bLimitFPS = true;
    this->bShowFPS = false;
    this->eRenderer = Direct3D;
}

void ControlsSettings::LoadDefaultValues()
{
    this->dFwdBackSecs = 3.0;
    this->dSpeedUpPct = 10.0;
}

void SongLibrary::LoadDefaultValues()
{
    TCHAR sPath[MAX_PATH];
    if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, sPath ) ) )
        AddSource( sPath, Folder, false );
    if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, sPath ) ) )
        AddSource( sPath, Folder, false );
    _tcscat_s( sPath, TEXT( "\\Piano From Above" ) );
    AddSource( sPath, FolderWSubdirs, false );
    if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, sPath ) ) )
        AddSource( sPath, Folder, false );
    m_bAlwaysAdd = false;
    m_iSortCol = 1; // File
}

void PlaybackSettings::LoadDefaultValues()
{
    this->m_ePlayMode = GameState::Splash;
    this->m_bMute = false;
    this->m_bPlayable = false;
    this->m_bPaused = true;
    this->m_dSpeed = 1.0;
    this->m_dNSpeed = 1.0;
    this->m_dVolume = 1.0;
}

void ViewSettings::LoadDefaultValues()
{
    this->m_bLibrary = true;
    this->m_bControls = true;
    this->m_bKeyboard = true;
    this->m_bNoteLabels = false;
    this->m_bOnTop = false;
    this->m_bFullScreen = false;
    this->m_bZoomMove = false;
    this->m_fOffsetX = 0.0f;
    this->m_fOffsetY = 0.0f;
    this->m_fZoomX = 1.0f;
    this->m_iMainLeft = CW_USEDEFAULT;
    this->m_iMainTop = CW_USEDEFAULT;
    this->m_iMainWidth = 960;
    this->m_iMainHeight = 589;
    this->m_iLibWidth = 0;
}

void AudioSettings::LoadMIDIDevices()
{
    wstring oldOutDev( this->iOutDevice >= 0 ? this->vMIDIOutDevices[this->iOutDevice] : L"" );
    this->iOutDevice = -1;
    this->vMIDIOutDevices.clear();
    int iNumOutDevs = midiOutGetNumDevs();
    for ( int i = 0; i < iNumOutDevs; i++ )
    {
        MIDIOUTCAPS moc;
        midiOutGetDevCaps( i, &moc, sizeof( MIDIOUTCAPS ) );
        this->vMIDIOutDevices.push_back( moc.szPname );

        if ( this->sDesiredOut == this->vMIDIOutDevices[i] )
            this->iOutDevice = i;
        if ( oldOutDev == this->vMIDIOutDevices[i] && this->iOutDevice < 0 )
            this->iOutDevice = i;
    }
    if ( this->iOutDevice < 0 )
        this->iOutDevice = iNumOutDevs - 1;
}

//-----------------------------------------------------------------------------
// LoadConfigValues
//-----------------------------------------------------------------------------

void VisualSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txVisual = txRoot->FirstChildElement( "Visual" );
    if ( !txVisual ) return;

    // Attributes
    int iAttrVal;
    if ( txVisual->QueryIntAttribute( "KeysShown", &iAttrVal ) == TIXML_SUCCESS )
        this->eKeysShown = static_cast< KeysShown >( iAttrVal );
    if ( txVisual->QueryIntAttribute( "AlwaysShowControls", &iAttrVal ) == TIXML_SUCCESS )
        this->bAlwaysShowControls = ( iAttrVal != 0 );
    if ( txVisual->QueryIntAttribute( "AssociateFiles", &iAttrVal ) == TIXML_SUCCESS )
        this->bAssociateFiles = ( iAttrVal != 0 );
    if ( txVisual->QueryIntAttribute( "RenderMode", &iAttrVal ) == TIXML_SUCCESS )
        this->eRenderMode = static_cast< RenderMode >( iAttrVal );
    if ( txVisual->QueryIntAttribute( "Accidentals", &iAttrVal ) == TIXML_SUCCESS )
        this->eAccidentals = static_cast< Accidentals >( iAttrVal );
    txVisual->QueryIntAttribute( "FirstKey", &this->iFirstKey );
    txVisual->QueryIntAttribute( "LastKey", &this->iLastKey );

    //Colors
    int r, g, b, i = 0;
    TiXmlElement *txColors = txVisual->FirstChildElement( "Colors" );
    if ( txColors )
        for ( TiXmlElement *txColor = txColors->FirstChildElement( "Color" );
              txColor && i < sizeof( this->colors ) / sizeof( this->colors[0] );
              txColor = txColor->NextSiblingElement( "Color" ), i++ )
            if ( txColor->QueryIntAttribute( "R", &r ) == TIXML_SUCCESS &&
                 txColor->QueryIntAttribute( "G", &g ) == TIXML_SUCCESS &&
                 txColor->QueryIntAttribute( "B", &b ) == TIXML_SUCCESS )
                this->colors[i] = ( ( r & 0xFF ) << 0 ) | ( ( g & 0xFF ) << 8 ) | ( ( b & 0xFF ) << 16 );
    
    TiXmlElement *txBkgColor = txVisual->FirstChildElement( "BkgColor" );
    if ( txBkgColor )
        if ( txBkgColor->QueryIntAttribute( "R", &r ) == TIXML_SUCCESS &&
             txBkgColor->QueryIntAttribute( "G", &g ) == TIXML_SUCCESS &&
             txBkgColor->QueryIntAttribute( "B", &b ) == TIXML_SUCCESS )
            this->iBkgColor = ( ( r & 0xFF ) << 0 ) | ( ( g & 0xFF ) << 8 ) | ( ( b & 0xFF ) << 16 );
}

void AudioSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txAudio = txRoot->FirstChildElement( "Audio" );
    if ( !txAudio ) return;

    string sMIDIOutDevice;
    if ( txAudio->QueryStringAttribute( "MIDIOutDevice", &sMIDIOutDevice ) == TIXML_SUCCESS )
    {
        this->sDesiredOut = Util::StringToWstring( sMIDIOutDevice );
        for ( size_t i = 0; i < this->vMIDIOutDevices.size(); i++ )
            if ( this->vMIDIOutDevices[i] == this->sDesiredOut )
                this->iOutDevice = (int)i;
    }
}

void VideoSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txVideo = txRoot->FirstChildElement( "Video" );
    if ( !txVideo ) return;

    int iAttrVal;
    if ( txVideo->QueryIntAttribute( "ShowFPS", &iAttrVal ) == TIXML_SUCCESS )
        this->bShowFPS = ( iAttrVal != 0 );
    if ( txVideo->QueryIntAttribute( "LimitFPS", &iAttrVal ) == TIXML_SUCCESS )
        this->bLimitFPS = ( iAttrVal != 0 );
    if ( txVideo->QueryIntAttribute( "OpaqueStatus", &iAttrVal ) == TIXML_SUCCESS )
        this->bOpaqueStatus = ( iAttrVal != 0 );
    if ( txVideo->QueryIntAttribute( "Renderer", &iAttrVal ) == TIXML_SUCCESS )
        this->eRenderer = static_cast< Renderer >( iAttrVal );
}

void ControlsSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txControls = txRoot->FirstChildElement( "Controls" );
    if ( !txControls ) return;

    txControls->QueryDoubleAttribute( "FwdBackSecs", &this->dFwdBackSecs );
    txControls->QueryDoubleAttribute( "SpeedUpPct", &this->dSpeedUpPct );
}

void SongLibrary::LoadConfigValues( TiXmlElement *txRoot )
{
    LoadMetaData();

    TiXmlElement *txLibrary = txRoot->FirstChildElement( "Library" );
    if ( !txLibrary ) return;

    int iAttrVal;
    if ( txLibrary->QueryIntAttribute( "AlwaysAdd", &iAttrVal ) == TIXML_SUCCESS )
        m_bAlwaysAdd = ( iAttrVal != 0 );
    txLibrary->QueryIntAttribute( "SortCol", &m_iSortCol );

    TiXmlElement *txSources = txLibrary->FirstChildElement( "Sources" );
    if ( txSources )
    {
        m_mSources.clear();
        int iSourceType;
        string sSource;
        for ( TiXmlElement *txSource = txSources->FirstChildElement( "Source" ); txSource;
              txSource = txSource->NextSiblingElement( "Source" ) )
            if ( txSource->QueryStringAttribute( "Name", &sSource ) == TIXML_SUCCESS &&
                 txSource->QueryIntAttribute( "Type", &iSourceType ) == TIXML_SUCCESS )
                AddSource( Util::StringToWstring( sSource ), static_cast< Source >( iSourceType ), false );
    }
}

void SongLibrary::LoadMetaData()
{
    string sPath = Config::GetFolder();
    if ( sPath.length() == 0 ) return;

    // Open the file
    ifstream ifs( sPath + "\\MetaData.pb", ios::in | ios::binary | ios::ate );
    if ( !ifs.is_open() )
        return;

    // Read it all in
    int iSize = static_cast<int>( ifs.tellg() );
    char *pcMemBlock = new char[iSize];
    ifs.seekg( 0, ios::beg );
    ifs.read( pcMemBlock, iSize );
    ifs.close();

    m_Data.ParseFromArray( pcMemBlock, iSize );
    delete[] pcMemBlock;

    // Create the file maps
    for ( int i = 0; i < m_Data.file_size(); i++ )
    {
        PFAData::File *file = m_Data.mutable_file( i );
        m_mMD5s[ pair< string, int >( file->filename(), file->filesize() ) ] = file;
    }
    for ( int i = 0; i < m_Data.fileinfo_size(); i++ )
    {
        PFAData::FileInfo *fileInfo = m_Data.mutable_fileinfo( i );
        m_mFileInfos[ fileInfo->info().md5() ] = i;
    }
}

void PlaybackSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txPlayback = txRoot->FirstChildElement( "Playback" );
    if ( !txPlayback ) return;

    int iAttrVal;
    if ( txPlayback->QueryIntAttribute( "Mute", &iAttrVal ) == TIXML_SUCCESS )
        m_bMute = ( iAttrVal != 0 );
    txPlayback->QueryDoubleAttribute( "NoteSpeed", &m_dNSpeed);
    txPlayback->QueryDoubleAttribute( "Volume", &m_dVolume );
}

void ViewSettings::LoadConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txView = txRoot->FirstChildElement( "View" );
    if ( !txView ) return;

    int iAttrVal;
    if ( txView->QueryIntAttribute( "Library", &iAttrVal ) == TIXML_SUCCESS )
        m_bLibrary = ( iAttrVal != 0 );
    if ( txView->QueryIntAttribute( "Controls", &iAttrVal ) == TIXML_SUCCESS )
        m_bControls = ( iAttrVal != 0 );
    if ( txView->QueryIntAttribute( "Keyboard", &iAttrVal ) == TIXML_SUCCESS )
        m_bKeyboard = ( iAttrVal != 0 );
    if ( txView->QueryIntAttribute( "NoteLabels", &iAttrVal ) == TIXML_SUCCESS )
        m_bNoteLabels = ( iAttrVal != 0 );
    if ( txView->QueryIntAttribute( "OnTop", &iAttrVal ) == TIXML_SUCCESS )
        m_bOnTop = ( iAttrVal != 0 );
    txView->QueryFloatAttribute( "OffsetX", &m_fOffsetX );
    txView->QueryFloatAttribute( "OffsetY", &m_fOffsetY );
    txView->QueryFloatAttribute( "ZoomX", &m_fZoomX );
    txView->QueryIntAttribute( "MainLeft", &m_iMainLeft );
    txView->QueryIntAttribute( "MainTop", &m_iMainTop );
    txView->QueryIntAttribute( "MainWidth", &m_iMainWidth );
    txView->QueryIntAttribute( "MainHeight", &m_iMainHeight );
    txView->QueryIntAttribute( "LibWidth", &m_iLibWidth );
}

//-----------------------------------------------------------------------------
// SaveConfigValues
//-----------------------------------------------------------------------------

bool VisualSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txVisual = new TiXmlElement( "Visual" );
    txRoot->LinkEndChild( txVisual );
    txVisual->SetAttribute( "KeysShown", this->eKeysShown );
    txVisual->SetAttribute( "AlwaysShowControls", this->bAlwaysShowControls );
    txVisual->SetAttribute( "AssociateFiles", this->bAssociateFiles );
    txVisual->SetAttribute( "RenderMode", this->eRenderMode );
    txVisual->SetAttribute( "Accidentals", this->eAccidentals );

    if ( this->eKeysShown == VisualSettings::All2 )
    {
        this->iFirstKey = MIDI::CM1;
        this->iLastKey = MIDI::G9;
    }
    txVisual->SetAttribute( "FirstKey", this->iFirstKey );
    txVisual->SetAttribute( "LastKey", this->iLastKey );

    TiXmlElement *txColors = new TiXmlElement( "Colors" );
    txVisual->LinkEndChild( txColors );
    for ( int i = 0; i < sizeof( this->colors ) / sizeof( this->colors[0] ); i++ )
    {
        TiXmlElement *txColor = new TiXmlElement( "Color" );
        txColors->LinkEndChild( txColor );
        txColor->SetAttribute( "R", ( this->colors[i] >>  0 ) & 0xFF );
        txColor->SetAttribute( "G", ( this->colors[i] >>  8 ) & 0xFF );
        txColor->SetAttribute( "B", ( this->colors[i] >> 16 ) & 0xFF );
    }

    TiXmlElement *txBkgColor = new TiXmlElement( "BkgColor" );
    txVisual->LinkEndChild( txBkgColor );
    txBkgColor->SetAttribute( "R", ( this->iBkgColor >>  0 ) & 0xFF );
    txBkgColor->SetAttribute( "G", ( this->iBkgColor >>  8 ) & 0xFF );
    txBkgColor->SetAttribute( "B", ( this->iBkgColor >> 16 ) & 0xFF );

    return true;
}

bool AudioSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txAudio = new TiXmlElement( "Audio" );
    txRoot->LinkEndChild( txAudio );

    if ( this->sDesiredOut.length() > 0 )
        txAudio->SetAttribute( "MIDIOutDevice", Util::WstringToString( this->sDesiredOut ) );

    return true;
}

bool VideoSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txVideo = new TiXmlElement( "Video" );
    txRoot->LinkEndChild( txVideo );
    txVideo->SetAttribute( "Renderer", this->eRenderer );
    txVideo->SetAttribute( "ShowFPS", this->bShowFPS );
    txVideo->SetAttribute( "LimitFPS", this->bLimitFPS );
    txVideo->SetAttribute( "OpaqueStatus", this->bOpaqueStatus );
    return true;
}

bool ControlsSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txControls = new TiXmlElement( "Controls" );
    txRoot->LinkEndChild( txControls );
    txControls->SetDoubleAttribute( "FwdBackSecs", this->dFwdBackSecs );
    txControls->SetDoubleAttribute( "SpeedUpPct", this->dSpeedUpPct );
    return true;
}

bool SongLibrary::SaveConfigValues( TiXmlElement *txRoot )
{
    bool bSaved = SaveMetaData();

    TiXmlElement *txLibrary = new TiXmlElement( "Library" );
    txRoot->LinkEndChild( txLibrary );
    txLibrary->SetAttribute( "AlwaysAdd", m_bAlwaysAdd );
    txLibrary->SetAttribute( "SortCol", m_iSortCol );

    TiXmlElement *txSources = new TiXmlElement( "Sources" );
    txLibrary->LinkEndChild( txSources );
    for ( map< wstring, Source >::const_iterator it = m_mSources.begin(); it != m_mSources.end(); ++it )
    {
        TiXmlElement *txSource = new TiXmlElement( "Source" );
        txSources->LinkEndChild( txSource );
        txSource->SetAttribute( "Name", Util::WstringToString( it->first ) );
        txSource->SetAttribute( "Type", it->second );
    }
    return bSaved;
}

bool SongLibrary::SaveMetaData()
{
    string sPath = Config::GetFolder();
    if ( sPath.length() == 0 ) return false;

    string sData;
    m_Data.SerializeToString( &sData );

    ofstream ofs( sPath + "\\MetaData.pb", ios::out | ios::binary );
    if ( !ofs.is_open() ) return false;

    ofs << sData;
    ofs.close();
    return true;
}

bool PlaybackSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txPlayback = new TiXmlElement( "Playback" );
    txRoot->LinkEndChild( txPlayback );
    txPlayback->SetAttribute( "Mute", m_bMute );
    txPlayback->SetDoubleAttribute( "Volume", m_dVolume );
    txPlayback->SetDoubleAttribute( "NoteSpeed", m_dNSpeed );
    return true;
}

bool ViewSettings::SaveConfigValues( TiXmlElement *txRoot )
{
    TiXmlElement *txView = new TiXmlElement( "View" );
    txRoot->LinkEndChild( txView );
    txView->SetAttribute( "Library", m_bLibrary );
    txView->SetAttribute( "Controls", m_bControls );
    txView->SetAttribute( "Keyboard", m_bKeyboard );
    txView->SetAttribute( "NoteLabels", m_bNoteLabels );
    txView->SetAttribute( "OnTop", m_bOnTop );
    txView->SetDoubleAttribute( "OffsetX", m_fOffsetX );
    txView->SetDoubleAttribute( "OffsetY", m_fOffsetY );
    txView->SetDoubleAttribute( "ZoomX", m_fZoomX );
    txView->SetAttribute( "MainLeft", m_iMainLeft );
    txView->SetAttribute( "MainTop", m_iMainTop );
    txView->SetAttribute( "MainWidth", m_iMainWidth );
    txView->SetAttribute( "MainHeight", m_iMainHeight );
    txView->SetAttribute( "LibWidth", m_iLibWidth );
    return true;
}

//-----------------------------------------------------------------------------
// Library functions
//-----------------------------------------------------------------------------

int SongLibrary::AddSource( const wstring &sSource, Source eSource, bool bExpand )
{
    int iChanged = 0;
    map< wstring, Source >::iterator pos = m_mSources.find( sSource );
    if ( pos != m_mSources.end() )
    {
        if ( pos->second == eSource ) return 0;
        else iChanged = RemoveSource( sSource );
    }

    if ( bExpand ) iChanged += ExpandSource( sSource, eSource );
    m_mSources[sSource] = eSource;

    return iChanged;
}

int SongLibrary::RemoveSource( const wstring &sSource )
{
    int iRemoved = 0;
    map< wstring, vector< PFAData::File* >* >::iterator itSource = m_mFiles.find( sSource );
    if ( itSource != m_mFiles.end() )
    {
        vector< PFAData::File* > *pvFiles = itSource->second;
        iRemoved = (int)pvFiles->size();
        pvFiles->clear();
        m_mFiles.erase( itSource );
        delete pvFiles;
    }

    m_mSources.erase( sSource );
    return iRemoved;
}

int SongLibrary::ExpandSources()
{
    int iExpanded = 0;
    for ( map< wstring, Source >::const_iterator it = m_mSources.begin(); it != m_mSources.end(); ++it )
        iExpanded += ExpandSource( it->first, it->second );
    return iExpanded;
}

void SongLibrary::clear()
{
    for ( map< wstring, vector< PFAData::File* >* >::iterator itSource = m_mFiles.begin(); itSource != m_mFiles.end(); ++itSource )
    {
        vector< PFAData::File* > *pvFiles = itSource->second;
        pvFiles->clear();
        delete pvFiles;
    }
    m_mFiles.clear();
    m_mSources.clear();
    m_mMD5s.clear();
    m_mFileInfos.clear();
    m_Data.Clear();
}

int SongLibrary::ExpandSource( const wstring &sSource, Source eSource )
{
    TCHAR buf[1024];
    vector< PFAData::File* > *pvFiles = new vector< PFAData::File* >();

    int iExpanded = ExpandSource( TEXT( "\\\\?\\" ) + sSource, eSource, pvFiles, buf );
    if ( iExpanded > 0 ) m_mFiles[sSource] = pvFiles;
    else delete pvFiles;

    return iExpanded;
}

// Passing the buffer around avoids having to put it on the recursive stack
int SongLibrary::ExpandSource( const wstring &sPath, Source eSource, vector< PFAData::File* > *pvFiles, wchar_t buf[1024] )
{
    if ( eSource == File )
    {
        if ( m_ProgressCallback )
        {
            wstring sFilename = sPath.compare(0, 4, L"\\\\?\\") == 0 ? sPath.substr(4) : sPath;
            m_ProgressCallback( ++m_iFilesScanned, sFilename );
        }

        PFAData::File *pInfo = AddFile( sPath );
        if ( !pInfo ) return 0;
        pvFiles->push_back( pInfo );
        return 1;
    }
    else
    {
        // Copy the file to the buffer
        if ( sPath.length() + 7 > 1024 ) return 0;
        memcpy( buf, sPath.c_str(), sPath.length() * sizeof( wchar_t ) );
        memcpy( buf + sPath.length(), L"\\*.mid", 7 * sizeof( wchar_t ) );

        int iExpanded = 0;
        WIN32_FIND_DATA ffd;
        HANDLE hFind = FindFirstFile( buf, &ffd );
        if ( hFind != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ( m_ProgressCallback )
                    m_ProgressCallback( ++m_iFilesScanned, ffd.cFileName );

                PFAData::File *pInfo = AddFile( sPath + L'\\' + ffd.cFileName );
                if ( pInfo )
                {
                    pvFiles->push_back( pInfo );
                    iExpanded++;
                }
            }
            while ( FindNextFile( hFind, &ffd ) );
            FindClose( hFind );
        }

        if ( eSource == FolderWSubdirs )
        {
            buf[sPath.length() + 2] = L'\0';
            hFind = FindFirstFile( buf, &ffd );
            if ( hFind == INVALID_HANDLE_VALUE ) return iExpanded;

            do
            {
                if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                     wcscmp( ffd.cFileName, TEXT( ".." ) ) && wcscmp( ffd.cFileName, TEXT( "." ) ) )
                    iExpanded += ExpandSource( sPath + L'\\' + ffd.cFileName, eSource, pvFiles, buf );
            }
            while ( FindNextFile( hFind, &ffd ) );
            FindClose( hFind );
        }
    
        return iExpanded;
    }
}

int SongLibrary::CountFilesInSource()
{
    int iTotalFiles = 0;
    TCHAR buf[1024];

    for (const auto& source : m_mSources)
    {
        if (source.first.length() + 7 > 1024) continue;
        memcpy(buf, source.first.c_str(), source.first.length() * sizeof(wchar_t));
        memcpy(buf + source.first.length(), L"\\*.mid", 7 * sizeof(wchar_t));

        WIN32_FIND_DATA ffd;
        HANDLE hFind = FindFirstFile(buf, &ffd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    iTotalFiles++;
            } while (FindNextFile(hFind, &ffd));
            FindClose(hFind);
        }
    }

    return iTotalFiles;
}

PFAData::File* SongLibrary::AddFile( const wstring &wsFilename, MIDI *pMidi )
{
    // Does it exist? Prob should remove from map if it's there.
    WIN32_FILE_ATTRIBUTE_DATA fad{};
    GetFileAttributesEx( wsFilename.c_str(), GetFileExInfoStandard, &fad );
    if ( fad.dwFileAttributes == INVALID_FILE_ATTRIBUTES ) return NULL;

    // Protocol buffers don't take wstrings...
    string sFilename = Util::WstringToString( wsFilename.substr( 4 ) );

    // Is it already there?
    pair< string, int > fileLookup( sFilename, fad.nFileSizeLow );
    map< pair< string, int >, PFAData::File* >::const_iterator itFile =
        m_mMD5s.find( fileLookup );
    if ( itFile != m_mMD5s.end() ) return itFile->second;

    // No meta data for the file exists yet. Parse the file, unless already parsed
    bool bIsNew = false;
    if ( !pMidi )
    {
        pMidi = new MIDI( wsFilename );
        bIsNew = true;
    }
    if ( !pMidi->IsValid() )
    {
        if ( bIsNew ) delete pMidi;
        return NULL;
    }

    // Create file info
    PFAData::File *file = m_Data.add_file();
    file->set_filename( sFilename );
    file->set_filesize( fad.nFileSizeLow );
    m_mMD5s[ fileLookup ] = file;

    // Do we already have data for this file
    const MIDI::MIDIInfo &mInfo = pMidi->GetInfo();
    map< string, int >::const_iterator itFileInfo = m_mFileInfos.find( mInfo.sMd5 );
    if ( itFileInfo != m_mFileInfos.end() ) 
    {
        file->set_infopos( itFileInfo->second );
        if ( bIsNew ) delete pMidi;
        return file;
    }

    // We don't. Add the data to the lookup and the buffer
    if ( bIsNew ) pMidi->PostProcess();
    PFAData::FileInfo* fileInfo = m_Data.add_fileinfo();
    PFAData::SongInfo* songInfo = fileInfo->mutable_info();
    songInfo->set_md5( mInfo.sMd5 );
    songInfo->set_division( mInfo.iDivision );
    songInfo->set_notes( mInfo.iNoteCount );
    songInfo->set_beats( mInfo.iTotalBeats );
    songInfo->set_seconds( static_cast< int >( mInfo.llTotalMicroSecs / 1000000 ) );
    songInfo->set_tracks( mInfo.iNumChannels );
    m_mFileInfos[ mInfo.sMd5 ] = m_Data.fileinfo_size() - 1;

    file->set_infopos( m_Data.fileinfo_size() - 1 );
    if ( bIsNew ) delete pMidi;
    return file;
}

wstring VideoSettings::LoadGPUPreference() const
{
    HKEY hKey;
    wstring wsResult;
    if ( RegOpenKeyEx(
        HKEY_CURRENT_USER,
        GPUREGISTRYPATH,
        0,
        KEY_READ,
        &hKey ) == ERROR_SUCCESS )
    {
        DWORD dwType = 0;
        DWORD dwSize = 0;
        wstring wsOwnPath = Util::GetSelfPath();
        if ( RegQueryValueEx(
            hKey,
            wsOwnPath.c_str(),
            NULL,
            &dwType,
            NULL,
            &dwSize ) == ERROR_SUCCESS && dwType == REG_SZ )
        {
            vector< wchar_t > buffer( dwSize / sizeof( wchar_t ) );
            if ( RegQueryValueEx(
                hKey,
                wsOwnPath.c_str(),
                NULL,
                NULL,
                reinterpret_cast< LPBYTE >( buffer.data() ),
                &dwSize ) == ERROR_SUCCESS )
            {
                wsResult.assign( buffer.data(), dwSize / sizeof( wchar_t ) );
                if ( !wsResult.empty() && wsResult.back() == L'\0' )
                    wsResult.pop_back();
            }
        }
        RegCloseKey( hKey );
    }
    return wsResult;
}

bool VideoSettings::ParseGPUPreferenceString( const wstring &wsPrefString, UINT &uiVdrID, UINT &uiDevID, UINT &uiSubSysID ) const
{
    // wsPrefString = "SpecificAdapter=8086&A7A8&171E1025;GpuPreference=1073741824;"
    const wstring wsPrefix = L"SpecificAdapter=";

    // -> SpecificAdapter=8086&A7A8&171E1025;GpuPreference=1073741824;
    size_t szStart = wsPrefString.find( wsPrefix );
    if ( szStart == wstring::npos ) return false;
    szStart += wsPrefix.length();

    // SpecificAdapter=8086&A7A8&171E1025; <-
    size_t szEnd = wsPrefString.find( L';', szStart );
    if ( szEnd == wstring::npos ) return false;

    // 8086&A7A8&171E1025
    wstring wsAdapter = wsPrefString.substr( szStart, szEnd - szStart );

    // ...086&A7A...
    size_t szPos1 = wsAdapter.find( L'&' ); if ( szPos1 == wstring::npos ) return false;
    // ...A7A8&171E...
    size_t szPos2 = wsAdapter.find( L'&', szPos1 + 1 ); if ( szPos2 == wstring::npos ) return false;

    wstring wsVdrID = wsAdapter.substr( 0, szPos1 );  // "8086"
    wstring wsDevID = wsAdapter.substr( szPos1 + 1, szPos2 - szPos1 - 1 ); // "A7A8"
    wstring wsSubSysID = wsAdapter.substr( szPos2 + 1 ); // "171E1025"

    try
    {
        uiVdrID = stoul( wsVdrID, nullptr, 16 ); // "8086" -> 0x8086
        uiDevID = stoul( wsDevID, nullptr, 16 ); // "A7A8" -> 0xA7A8
        uiSubSysID = stoul( wsSubSysID, nullptr, 16 ); // "171E1025" -> 0x171E1025
    }
    catch ( const exception& )
    {
        return false;
    }

    return true;
}

bool VideoSettings::SaveGPUPreference( UINT uiVdrID, UINT uiDevID, UINT uiSubSysID ) const
{
    HKEY hKey;
    bool bResult = false;
    if ( RegCreateKeyEx(
        HKEY_CURRENT_USER,
        GPUREGISTRYPATH,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        NULL,
        &hKey,
        NULL ) == ERROR_SUCCESS )
    {
        wchar_t wcValue[256];
        swprintf_s(
            wcValue,
            size( wcValue ),
            L"SpecificAdapter=%04X&%04X&%08X;GpuPreference=1073741824;", uiVdrID, uiDevID, uiSubSysID
        );
        wstring wsOwnPath = Util::GetSelfPath();
        if ( RegSetValueEx(
            hKey,
            wsOwnPath.c_str(),
            0,
            REG_SZ,
            reinterpret_cast< const BYTE* >( wcValue ),
            (DWORD)( ( wcslen( wcValue ) + 1 ) * sizeof( wchar_t ) ) ) == ERROR_SUCCESS )
        {
            bResult = true;
        }
        RegCloseKey( hKey );
    }
    return bResult;
}
