
//
//		HSP compile/package functions for HSP3
//				onion software/onitama 2002
//

#include <stdio.h>
#include <emscripten.h>
#include <emscripten/bind.h>

#include "../../hsp3/hsp3config.h"

#include "../../hsp3/hsp3debug.h"			// hsp3 error code
#include "../../hsp3/hsp3struct.h"			// hsp3 core define
#include "../../hsp3/hspwnd.h"				// hsp3 windows define

#include "../supio.h"
#include "../hsc3.h"
#include "../token.h"
#include "../ahtobj.h"

#define DPM_SUPPORT		// DPMファイルマネージャをサポート
#include "../win32dll/dpm.h"

static char fname[HSP_MAX_PATH];
static char rname[HSP_MAX_PATH];
static char oname[HSP_MAX_PATH];
static char hspexe[HSP_MAX_PATH];
static int opt1,opt2,opt3;

static int orgcompath=0;
static char compath[HSP_MAX_PATH];

static CHsc3 *hsc3=NULL;

extern char *hsp_prestr[];

int main()
{
	Alertf("%s%s / Emscripten %d.%d.%d", HSPTITLE, hspver, __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__);
	hsc3 = new CHsc3;
	return 0;
}

//----------------------------------------------------------

static int GetFilePath( char *bname )
{
	//		フルパス名から、ファイルパスの取得(\を残す)
	//
	int a,b,len;
	char a1;
	b=-1;
	len=strlen(bname);
	for(a=0;a<len;a++) {
		a1=bname[a];
		if (a1=='/') b=a;
		if (a1<0) a++; 
	}
	if (b<0) return 1;
	bname[b+1]=0;
	return 0;
}

//----------------------------------------------------------

void hsc_ini ( const std::string p1 )
{
	strcpy(fname,p1.c_str());
	strcpy(rname,p1.c_str());
	strcpy(oname,p1.c_str());
	cutext(oname);
	strcat(oname,".ax");
}


void hsc_refname ( const std::string p1 )
{
	strcpy(rname,p1.c_str());
}


void hsc_objname ( const std::string p1 )
{
	strcpy(oname,p1.c_str());
}


std::string hsc_ver ()
{
	char ver[64];
	sprintf( ver, "%s ver%s", HSC3TITLE, hspver );
	return std::string( ver );
}


void hsc_bye ()
{
}


std::string hsc_getmes ()
{
	return std::string( hsc3->GetError() );
}


void hsc_clrmes ()
{
	hsc3->ResetError();
}


void hsc_compath ( const std::string p1 )
{
	strcpy(compath,p1.c_str());
	orgcompath=1;
}


int hsc_comp ( int p1, int p2, int p3 )
{
	int st;
	int ppopt;
	int cmpmode;
	char fname2[HSP_MAX_PATH];

	hsc3->ResetError();
	if (orgcompath==0) {
		strcpy( compath,"/common/" );
	}
	strcpy( fname2, fname );
	strcat( fname2, ".i" );
	hsc3->SetCommonPath( compath );
	ppopt = HSC3_OPT_UTF8IN;
	if ( p1 ) ppopt|=HSC3_OPT_DEBUGMODE;
	if ( p2&1 ) ppopt|=HSC3_OPT_NOHSPDEF;
	if ( p2&4 ) ppopt|=HSC3_OPT_MAKEPACK;
	if ( p2&8 ) ppopt|=HSC3_OPT_READAHT;
	if ( p2&16 ) ppopt|=HSC3_OPT_MAKEAHT;
	st = hsc3->PreProcess( fname, fname2, ppopt, rname );
	if ( st != 0 ) {
		hsc3->PreProcessEnd();
		return st;
	}
	if ( p1 == 2 ) {
		hsc3->PreProcessEnd();
		return 0;
	}

	cmpmode = p1 & HSC3_MODE_DEBUG;
	cmpmode |= HSC3_MODE_UTF8;
	if ( p3 ) cmpmode |= HSC3_MODE_DEBUGWIN;
	st = hsc3->Compile( fname2, oname, cmpmode );
	hsc3->PreProcessEnd();
	if ( st != 0 ) return st;
	return 0;
}


//----------------------------------------------------------

void pack_ini ( const std::string p1 )
{
#ifdef DPM_SUPPORT
	strcpy(fname,p1.c_str());
	cutext(fname);
	if ( hsc3==NULL ) Alert( "#No way." );
	hsc3->ResetError();
	dpmc_ini( hsc3->errbuf, fname );
	opt1=640;opt2=480;opt3=0;
	strcpy(hspexe,"hsprt");
#endif
}


int pack_view ()
{
	int st;
#ifdef DPM_SUPPORT
	st = dpmc_view();
#else
	st = 0;
#endif
	return -st;
}


int pack_make ( int p1, int p2 )
{
	int st;
#ifdef DPM_SUPPORT
	if ( p2 != 0 ) dpmc_dpmkey( p2 );
	st=dpmc_pack(p1);
#else
	st = 0;
#endif
	return -st;
}


void pack_opt ( int p1, int p2, int p3 )
{
	opt1=p1;if (opt1==0) opt1=640;
	opt2=p2;if (opt2==0) opt2=480;
	opt3=p3;							// disp SW (1=blank window)
}


void pack_rt ( const std::string p1 )
{
	strcpy(hspexe,p1.c_str());
}


int pack_exe ( int p1 )
{
	int st;
#ifdef DPM_SUPPORT
	st=dpmc_mkexe(p1,hspexe,opt1,opt2,opt3);
#else
	st = 0;
#endif
	return -st;
}


int pack_get ( const std::string p1 )
{
	int st;
#ifdef DPM_SUPPORT
	char fn[HSP_MAX_PATH];
	strcpy(fn,p1.c_str());
	st=dpmc_get(fn);
#else
	st = 0;
#endif
	return -st;
}


//----------------------------------------------------------
//		Additional service on 2.6
//----------------------------------------------------------

int hsc3_getsym ( int p1 )
{
	hsc3->ResetError();
	if (orgcompath==0) {
		strcpy( compath,"/common/" );
	}
	hsc3->SetCommonPath( compath );
	if ( hsc3->GetCmdList( p1|2 ) ) return -1;
	return 0;
}


int hsc3_messize ()
{
	return hsc3->GetErrorSize();
}


int hsc3_make ( const std::string p1 )
{
	char libpath[HSP_MAX_PATH];
	int i,type;
	int opt3a,opt3b;
	int st;

	if ( hsc3==NULL ) Alert( "#No way." );
	hsc3->ResetError();

	strcpy( libpath, p1.c_str() );
	GetFilePath( libpath );

	i = hsc3->OpenPackfile();
	if (i) { Alert( "packfileが見つかりません" ); return -1; }
	hsc3->GetPackfileOption( hspexe, "runtime", "hsprt" );
	strcat( libpath, hspexe );
	strcpy( hspexe, libpath );
	hsc3->GetPackfileOption( fname, "name", "hsptmp" );
	cutext( fname );
	type = hsc3->GetPackfileOptionInt( "type", 0 );
	opt1 = hsc3->GetPackfileOptionInt( "xsize", 640 );
	opt2 = hsc3->GetPackfileOptionInt( "ysize", 480 );
	opt3a = hsc3->GetPackfileOptionInt( "hide", 0 );
	opt3b = hsc3->GetPackfileOptionInt( "orgpath", 0 );
	opt3 = 0;
	if ( opt3a ) opt3 |= 1;
	if ( opt3b ) opt3 |= 2;

	hsc3->ClosePackfile();

	//		exeを作成
#ifdef DPM_SUPPORT
	dpmc_ini( hsc3->errbuf, fname );
	st=dpmc_pack( 0 );
	if ( st ) return -st;
	st=dpmc_mkexe( type, hspexe, opt1, opt2, opt3 );
	strcat( fname, ".dpm" );
	delfile( fname );
#else
	st = 0;
#endif
	return -st;
}


//----------------------------------------------------------
//		Additional service on 3.0
//----------------------------------------------------------

std::string hsc3_getruntime ( const std::string p1 )
{
	char rt[HSP_MAX_PATH] = {};
	char fn[HSP_MAX_PATH];
	strcpy(fn,p1.c_str());
	hsc3->GetRuntimeFromHeader( fn, rt );
	return std::string( rt );
}


//----------------------------------------------------------
//		Export for Javascript
//----------------------------------------------------------

using namespace emscripten;

EMSCRIPTEN_BINDINGS ( hspcmp )
{
	function("hsc_ini", &hsc_ini);
	function("hsc_refname", &hsc_refname);
	function("hsc_objname", &hsc_objname);
	function("hsc_ver", &hsc_ver);
	function("hsc_bye", &hsc_bye);
	function("hsc_getmes", &hsc_getmes);
	function("hsc_clrmes", &hsc_clrmes);
	function("hsc_compath", &hsc_compath);
	function("hsc_comp", &hsc_comp);
	//function("pack_ini", &pack_ini);
	//function("pack_view", &pack_view);
	//function("pack_make", &pack_make);
	//function("pack_opt", &pack_opt);
	//function("pack_rt", &pack_rt);
	//function("pack_exe", &pack_exe);
	//function("pack_get", &pack_get);
	//function("hsc3_getsym", &hsc3_getsym);
	//function("hsc3_messize", &hsc3_messize);
	//function("hsc3_make", &hsc3_make);
	function("hsc3_getruntime", &hsc3_getruntime);
}
