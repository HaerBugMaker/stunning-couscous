//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
Tmainform *mainform;
//---------------------------------------------------------------------------
__fastcall Tmainform::Tmainform(TComponent* Owner)
	: TForm(Owner)
{
	std::vector<TMyIp> VC_MyIpPool;
	GetMyIP(VC_MyIpPool);
	auto cell = VC_MyIpPool.front();
	UDPSvr->Bindings->Add();
	UDPSvr->Bindings->Items[0]->IP = cell.IpAddr;
	UDPSvr->Active = true;
	Label1->Caption = UDPSvr->Bindings->Items[0]->IP;
}
//---------------------------------------------------------------------------
void __fastcall Tmainform::BitBtn1Click(TObject *Sender)
{
	std::vector<TBytes> VC_TBytes;
	RdPara(0,sizeof(TSysPara),VC_TBytes,TPkgIncrease::increase_1024);
	for(const auto &abyte:VC_TBytes){
		UDPSvr->Bindings->Items[0]->SendTo(LabeledEdit1->Text,2305,abyte);
	}
}
//---------------------------------------------------------------------------
void __fastcall Tmainform::UDPSvrUDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
		  TIdSocketHandle *ABinding)
{
	if(ABinding->PeerIP == LabeledEdit1->Text) ShowMessage(L"收到网络回复");
	const TCanTRDat* pCanTRDat = reinterpret_cast<const TCanTRDat*>(&AData[AData.Low]);
	//解析CAN包头 不对装置地址进行校验
	if(strcmp(pCanTRDat->Cmd,"CanRDat") == 0){
		 ShowMessage(L"连接成功");
		 return;
	}

	const TUDPSysPara* pUDPSysPara = reinterpret_cast<const TUDPSysPara*>(&AData[AData.Low]);
	if(strcmp(pUDPSysPara->Cmd,"WriteOK!") == 0) ShowMessage(L"下发成功");
	else {
		const TSysPara* pSysPara = reinterpret_cast<const TSysPara*>(pUDPSysPara->Para);
		//显示系统参数
		LabeledEdit2->Text = pSysPara->DirectTR;
		LabeledEdit3->Text = IPCharToStr(pSysPara->Hostip);
		LabeledEdit5->Text = pSysPara->CanBaud;
		LabeledEdit9->Text = MACCharToStr(pSysPara->Hostmac);
	}
}
//---------------------------------------------------------------------------
void __fastcall Tmainform::BitBtn3Click(TObject *Sender)
{
	std::vector<TBytes> VC_TBytes;
	char rst = 'r';
	WrPara(offsetof(TSysPara,FirstUse1),1,&rst,VC_TBytes);
	for(const auto &abyte:VC_TBytes){
		UDPSvr->Bindings->Items[0]->SendTo(LabeledEdit1->Text,2305,abyte);
	}
}
//---------------------------------------------------------------------------
void __fastcall Tmainform::GCS28xx_Rd_PMeasStr(u8 Add,TBytes &AByte)
{
	TCANPSOE   CANPSOE{};
	CANPSOE.TYP = 21;
	CANPSOE.VSQ = 0x81;
	CANPSOE.COT = 42;
	CANPSOE.ADD = Add;
	CANPSOE.FUN = 255;
	CANPSOE.INF = 241;
	TPCANITEM CANITEM{};
	CANITEM.GROUPID = 6;
	CANITEM.KOD = 10;

	int len = sizeof(CANPSOE) + sizeof(CANITEM);
	AByte.set_length(len);
	memcpy(&AByte[0],&CANPSOE,sizeof(CANPSOE));
	memcpy(&AByte[sizeof(CANPSOE)],&CANITEM,sizeof(CANITEM));
}

void __fastcall Tmainform::BitBtn4Click(TObject *Sender)
{
	TBytes AByte;             //待打包数据
	TBytes UDPBytes;          //UDP数据
	GCS28xx_Rd_PMeasStr(LabeledEdit4->Text.ToInt(),AByte);
	PackCan(LabeledEdit4->Text.ToInt(),AByte.Length,0,&AByte[0],UDPBytes);
	UDPSvr->Bindings->Items[0]->SendTo(LabeledEdit1->Text,LabeledEdit7->Text.ToInt(),UDPBytes);
}
//---------------------------------------------------------------------------

void __fastcall Tmainform::BitBtn5Click(TObject *Sender)
{
	u8 IP[4]{0};
	StrToIPChar(AnsiString(LabeledEdit6->Text).c_str(),IP);
	std::vector<TBytes> VC_TBytes;
	WrPara(offsetof(TSysPara,Hostip),4,IP,VC_TBytes);
	for(const auto &abyte:VC_TBytes){
		UDPSvr->Bindings->Items[0]->SendTo(LabeledEdit1->Text,2305,abyte);
	}
	char Mac[6]{0};
	StrToMac6(LabeledEdit8->Text,Mac);
	WrPara(offsetof(TSysPara,Hostmac),6,Mac,VC_TBytes);
	for(const auto &abyte:VC_TBytes){
		UDPSvr->Bindings->Items[0]->SendTo(LabeledEdit1->Text,2305,abyte);
	}
}
//---------------------------------------------------------------------------

