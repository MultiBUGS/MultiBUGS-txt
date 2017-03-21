	Setting up the development
						tools

Contents

	BlackBox
	TAUCS library
	Subversion repository

BlackBox	[top]

Download and install BlackBox component builder v1.6 from OberonMicrosystems (http://www.oberon.ch). 

OberonMicrosystems have kindly made some tools for developing ELF format shared object files available under the Sleepy Cat Open Source licence (http://www.opensource.org/licenses/sleepycat.php). 

Install OpenBUGS in some directory different from where you have installed BlackBox. If you have installed BlackBox in a directory in program files it will be convenient to install OpenBUGS in a directory in program files (say c:/Program Files/OpenBUGS). Next edit the short cut to BlackBox so that its target contains /USE "c:/Program Files/OpenBUGS" after the text BlackBox.exe.  Note the use of forward slashes as separators in file paths.  On some installations, it has been reported that backslashes are required instead, as in  /USE "c:\Program Files\OpenBUGS"

Next double click on the BlackBox icon on the desk top to start BlackBox. 

The BlackBox directory will also contain a functioning version of the OpenBUGS software and is a useful place to develope and test new software. If you do not want to do this just un-install BlackBox by clicking on the un-install icon.

The end user version of OpenBUGS for Windows contains a simplified trap handler. This trap handler is in file Bugs/Code/Traphandler.ocf. If you pefer to use Oberonmicrosystems extremely powerful trap handler just rename the Traphandler.odc file.

TAUCS library	[top]

The OpenBUGS can make use of the TAUCS sparse matrix library (http://www.tau.ac.il/~stoledo/taucs). A Windows version of this library, libtaucs.dll, is distributed with OpenBUGS. This library is used in a sampling algorithm for Gaussian Markov Random Field (GMRF). Without TAUCS the sampling algorithm for GMRF will be much slower for high dimension problems. If you do not want OpenBUGS to use the TAUCS library just remove libtaucs.dll from the OpenBUGS directory. The library version of OpenBUGS does not use the TAUCS library

We do not have Linux or C so we can not produce a Linux version of the TAUCS library. However the library is Open Source and can be downloaded from the web as source code. If you obtain a version of the TAUCS library in ELF format and  you wish to use TAUCS to do sparse matrix algebra in the brugs.so library add the name MathTaucsImp to the list of modules linked just after MathSparsematrix.   

Subversion repository	[top]

Source code of OpenBUGS is stored in the Subversion (SVN) repository at SourceForge (https://openbugs.svn.sourceforge.net/svnroot/openbugs). A fancier view of the repository is provided via ViewVC (http://openbugs.svn.sourceforge.net/viewvc/openbugs). Comunication with the SVN repository can be perfomred with any Subversion client. We recommend using the TortoiseSVN client (http://tortoisesvn.tigris.org).

BlackBox .odc files are stored as binary files, which complicates the actions usually performed with source files stored in text from. One of such tasks is generating the difference between two files. There are several ways to check for differences between two .odc files.

1. BlackBox can be configured to work with the TortoiseSVN interface to source code revision software. It is posible to write a simple java script to act as the "Diff" program for BlackBox .odc files. The basic idea is to have a modified Config file that causes BlackBox to open two files and then use the Compare procedure in module DevSearch to compare the files. The java script just passes the correct arguments to BlackBox. Place the java script program in "C:\Program Files\TortoiseSVN\Diff-Scripts" and place the code file of the special Config module in  "C:\Tortoise\DiffConfig\Code".

##=>java script diff program by josef temple, click on arrow and then use Tools->Decode to decode and place in correct location 
StdCoder.Decode YZaNapmPYBa9eB5.,0 eB5.bZ....Td8JN1JM0Pc0,d8,7JFPOb96bvMZPO
 V9R,NOb96KLs0rrCrmGKEGrr0GlKKEaKrCLu4KqmqmGKEaKr0GeyKtGrraqtKqdOJb0mkCLE4K
 ECquCLuyqq0GnaKqKKEGqoOKn0Gs8rrSKt4qq0GnyKt0mrGql0GnaKqQiigV82YeMGd96b9R1f
 QdvQ,dENPM5vO3vPl965uPP9QTfP9fPd963ORH9P7PNZ99,tPVPNRvQ,7RFPN,7RjvP,NMZvNf
 PP9fPd96BPONXg2YBIVUI3dPONPNb96d9OqKEOrm8Luaql4KqmqwmGE4KrGKECqrqKs4KtKqt0
 GuUrBho3hUwY7phaxhhob1xhh,,7J99SdvQDd9Pc0cFTfQ,tMTfPBPODPRZPORvN,7RFHECrl8
 rmKKr0Gq4qwyquGLE4KrMORfRTvOHfPD96EuKKwGLECqrUdRijphg2YdZiBIVUkuCrmCLE4Kr0
 mk0Ls8bkJidBgohgUwY1xhipASGEqqrGqumqm0GuWqkGLEaqt2iVRinhgYNO0GlmqkC4wYn3Yh
 ginhgUQggBgpRiZpZBIVUM1J660GbyKuKKR0GXaKqKKEuqkqqmCLEuqrGLEKKrGqouqn0mouKE
 SGLyKmCqH0mk8rmggs3iZRgohgY3Yo,GLtKqkGrmGKE8qw0GuWqmIcgBA8orWr4eEEsMT1KbVZ
 idxhi3YVRC38P.,7NTvMfPP9XnpZUQepRgc3YaBB.2hV3ikhgi3YoxhUAgk3C1fQ,NNRtNR76j
 9O9fPPc06FOKnAhn3YXBggZhZFn8rrqKEGLo6JTfQUUIiZ3ijRidZijJit3YWJijxinhgm3YVp
 gohgm3YXxhh,66TfN,7NHfQ9vMdvPZPO9vQRN1M166,OMf9RFvPZ96JuPbPNB96dONP9QNP1J6
 6cR9fQbPOTfP,NARdAPc0,lLqE3qE3Ork8LE8KqElMFlPNN767POBnVyKrEK0mk8rnCLK0GrKr
 qmGE8qkCrmyImCKK0WZxiDZgXZ3BvQN76b9O99PN99,dP9Hb4qqKqRqE3qUBIVjwZU2ighgVRi
 Z3YVZgV3io3Yo3hZRiZ3YkBgo3hZRiUsP,NSTPRZ969fPhPOZvPRPP9fPdP1JcMNPM.2Yx2YWQ
 cuwZEJijxAcFH9PCrL8Iq4qlI6Q6wB66307XUAaioajIcgBAELKKwKKFqE3GqoO4UUgbUI2Yej
 Jio,T7FHfNB1c6Pc0Pk4Agmxgn3Yx2YLReXJidFL4ItSquUnRbBIVihih3Yx2YVJibRiiYhZph
 bZicRbBIVdpgU2Zihih3Yw2YmAZUQjBIVU2YLReX,KolWqrWGFKpt4qnKKR0mfCplUUYgdlKyK
 mCKLeqt0Gl4qtoZjZgX3YihgrpZjZA3N8rN1J66.oZFhidZicAadQbBIVxhV8gV8IgVRiZx70m
 SAA2ZkAZvgV8ohZxiDtM,,kMamRM1Oqt0mS0GrKqv0mUCKuaKvKKgyIleqmCKuWGFCpl8roMOR
 vNRdFH9PCpwCLuKqqyIleqmc6HtCPkoEE4GEOqtuGXa498SHvQdvQFdM1vQkIaGEir4.Uig6EX
 EE8GEiGE8qk.,t8,d6,7NTPNb96R1WroCbi2YUQcVphixB,tMTXVNNGLo2YYxhXlmEL8mIin42
 2.A8.gDaKn..K2QCRPNjvHA3..g6.22cP910WW2YYxB.91u0.C4....,,.X0.v1.krmKmKKtKI
 wa4bHmaKnQ6kIaGE..g6EWaKtkrADDd6,t8,7NHfN.0mJ0Wb2YixBOqrKLrGKLmJr8m4eEE0GE
 0GE.s8,d6V8P9PMsMFPN5vO,dR1fQHPM39P996D7NHfNB1s7aKrY8G5Q8yGWaKngZHRgmBBCrL
 GqoO4TXiEFamR66S3EL43.q5.UdlW.U1,00ymVyKmKqLCoru4umrCKn8mIaGEi5.kWC4QcjpBg
 C.PvP71SGF..22..o32Yixho,.EE22.i0V8P44..kHG4.S0...PtIUn,wB.66S3EL43.q5BvQk
 umKqin4ek4YimBjUQjB66CLoEEqHESZX,CItKqkGbDJgeVcsJbuMZ1CJoK43N8rVU.oZ33jZRg
 cIggBA8466L76376PNRbHE8GE.CYiVU.Dd6,VW,EE0GFSGE0mH8GEcPUUEFSGFamR.EEcP.vXX
 BgoRgc3ZZBZUQD0WL,MF51aprKLEqquCLu0Go4KvKKESGFElmqk..2YdtQ.Grr2iZJiaxhmhhU
 YicBhntPVPNZPMELmJr8GEqE30GE0WU.kJ6INnkkl..s739P1vMkH0mouKE..UhQ8kt.y4R,.j
 0c9X0.v1Vm1...
 --- end of encoding ---##<=

##=>compiled form of config module by josef temp, click on arrow and then use Tools->Decode to decode and place in correct location
StdCoder.Decode 1xaPaZqNiwqMa,.,0 jBaN..kv....BuETuPJ.,Z.2W5EwF2H.0.TsETfPB
 PODnVyKrOrm8LuKKtCb2BhVZhjxAb8RTfQ9vQOoomqmCb6xhnZiBhgihintId9N7OO11OpoKqv
 CbHhgmpidRgZRCLONZfP99PGomOrdKqk8rlWaDZhZZcVZiVNHT9N99PbHcyKtGrtocjphoRCFu
 PYeZ3joRcjpB,Ey5EGGW2Yk4gUOgUIYzIYedZhZBcXZidxhipZ2xBvD26NF6RFPObn4uu.0oR5
 kZKi6hPFesioq3dY9G5LSOh6JQdux0JpZep9JVD.FusmnqhVIYVhYz3CVHhgohik3Ux5WWzzzb
 goZidZhZ,2.2.E,EfHT8WJ8EW4.2UYJWKLaXoi.zgEJW5EnGUYV...........sI99RfXIBhg,
 cLGpom4U.krYA5X.6.Yc.wB5nugB9HrYCbHLg6lHswBZHuY6QA.E.2.70..b1Hmq2CTHtEWkl.
 912.,EVYB1nlQB3mr2D,lYoBBnr2.TGqgA7mkYC1HLg6.Z1H0.,6.T0.d1RlYEs...0EV.51Ew
 EWAAd1,kb.EuELg6.EbAAdnooC91,6.T0.dnko3H0..AB.0Ee0km2DdHfAB9nvQCRFfABE.6.Z
 moQAFHEY8.,7.BmrICPnkYC,E.27TntYCd0kVwBRHvo390V1ZHuY6Z0Ee.dH.2.TGlIB9nlYC,
 FWgg.QC5HtABVHuwB2.TGqkVYBHnmoBdHLg62CEuA7oA2.0kbkm.H1YCRlY.T1A7oAU.w7gAko
 .dHLo8gAj10kWgB3nmYA7nmYA,lbIAklE.w7NnmQ6ABoBkW..,E.g6.71bmrgCZnlgA6.T09nd
 gAZHvgAZHL..E.w7kVko.o3H0V1E.2.F0b1d0lHuQ6oBo3H0EtEWI8QAF1.,E.g8RmYQ6TGWg6
 dmW29dG.27QCkm.50.90V1Z1g8RnokrYA9H.27QC.lHu.R1A7P1IC..YA2.dmW.,E.27QCkm2D
 .R1g62CICEe.dH.2.F0b1.Q6.A7.Z1.E.2.F0b1VmoQAdnuIC9nto3kmwCg79GeA6BmYY79Gc0
 kYQ6d0,EY.28.f1.90V1Z12e..2.,EYktEc.gC.A7P1IC..,E.27QCI6HHugB1HsQCo8gAE.0E
 VA7dmaA6VG.0E.27.ABd1.g6.Z1I6.E..b1..b1HmqkrIC..E..b1..b1.THt.V0kUQC3mo.E.
 w7gA70dnko390.28.f12.0...vP.E.Av5I.7k6Q,T,0...wV.U.k0HU.6.2D2.GK.0.EMXzzHK
 KXzzHK.E...5uPRfN6.njH6N,U....Gq,CV.2gzzzYxzzzzzXY3.OK42.N9yzr6S.0E.GKmWzz
 HK.6.........EAXzzHK2EwzzzzjznVLp3nSolmWCQlZjf0b.fuWNzJheOeq.CMW1STV7ywZio
 1as2KB1AluzzzTT,av4ktLu.Hgy1..F5W41k,HQ0f4..S.Cj,0.c0sw.UY,swWC,EpQk2bE.UY
 V5.46.U9....6u3ZZJB2ky060sw46.U7...s1.GEzzzzfacLRKKoM...x3k2.2..k3....2xmA
 .9fO3sU7CUx3E2.12.....H.E0EpMT1kme4GMp.oL.D.E...H....EoLw.gA..it.Yg7yQF.AE
 7.Jn.HAorzD.71H.2xBI.9fOUo9.71F.AE,.Wk.Yg7m2.Y.IBczor..w,EobI,gAUHJ.7PWDL2
 .1I...BU.QS.EmE5kt,.71kt3xFK.91sa3EmE2.12.7XZOd1Al.Uti,....FTt36.916..H.AE
 ..D...k4....2xpQ.9fO..H.AE..7U....P....Eobs,gAUHS.7PWDr2.12sI5.w..7.J1ET1.
 E62U.Eorx,gA6V.sy5Emk1.1I.U.Emk3.Y.IBFT776.99O2UY3Rgg7icoLBJnzLLuXCglZ3zSx
 fRRsE4mESx6U95VMjVL4VMiVDzUYbVvyUkaVbyUUaVbzUwTVvrU6TVbrUISVr4VsLVzqUILVbq
 UUKVHqUgDV1qUUDVLjUcCV1jUk5VjiUM5V1iUQ4VvaUszUfTUIzULTUsyUjSUIyUTaUorU5SUI
 rUjLUcqUPLUwjU5LUMjUQKUQiUbbUsbUjj.76r16f76m16U76h16t56Z16j56U16Z56vWT,ag2
 LUp2m0AY2KUECi.CUICicCU64iYCUQCiYDUcCCVjTlbNyj.CU25iQCUE5C,6rVP.3QQd6dXXUC
 CGEdKqnaqtGrm8b7LrYjstxOAUdhhkxhmZi,ZhgdqFAhZR,8kdGLtaKrSapirGDlwB.8EeaLsK
 Kb4qqKaY5weyrtB.8.2UGDxWru3A.GEeaKqoeZJioBhXBggdpHzo,S,9.,6.3ME59RHHrIyKzl
 trj0c.1uMEjgzfywOjU,2vkZj65CGEWyKa4KuKKtYmqixNqUu5,4EryqvIv6aK2U2QcjhhkBgm
 hAhSuba8U2I8.4Tu5Q2brkX.1MORfNTHJvvt9x.4ktGrr8rkSqmgxn4qIi.,6.0.Lp....
 --- end of encoding ---##<=

##=>source code for config file writen by josef temple
StdCoder.Decode 2ZaNatmPYB4.,0 1Ak...56,...58FTuPE,5TWyqlKrqKKrGrtumdGLmGor
 CquU2hgnRAXDFTvMUn7FTvMf1G2sETfPdPMHfP9fQbf9hOO9vR7ONbvMoedhgrRiioedFWUkTe
 oxhmhgnpZHZijJC7ONbvM0.,.S.Gs6U.QklbcjRAktgdjZgZZh2hgnlsDor.kay4.qorGqmQii
 g76FUHZijJCbnWmqmqKWKqtcw7.,.ZC2A,ItEE.0.8cIhgsZiKBhZxintId9NhOO9HWUlbeZ3D
 kto8Y6cw3.0.k1Eu0,,6.CcIhgsNHT9N9ntQ8qorG4704D.CbB,708T1U.Ezu.T.hf12.,.Ny.
 cU.ktAcoZimBhWhiohgnZcZRCY.2.Q32U.EDU.U,.1eQHPMNHyuv.U.2m,.b.1cUZT1E.sB6.,
 US.,Uz1,ME..,6YUBU,2.0k6Tx5QXiI.3Qwb8R7vIdPMP9Qbf9bWK,U1.,U1C.6.Kr.U0w0ZRF
 dsIj.1z9LEUG.FPzhN3Z.dy9LEV8.xfZh8oD.by9LEAG.1StftL1.9y9LkGD.0E...........
 ........................................AU0U0Ikmj,2U.k8V.EBE.0.4E.EJYjyC.6
 .V2A.AUQV0A.6F.sJc.UM.U6V.AU.U,U0.1sI.MQc.k.8.AHIU5U.I,IUCU.o3QU0KyBU.2.K1
 2.u,0E.k.0.42.4.072U,U0I,,U9U2Ikmj,2U.kdE.0.x.0.4.A6.2.0ES9.g.c8M.F1,c62U0
 o,2UCU0.,M8M.B09cUZT16.,..u,0E.k.0.426sNQ.E.07MV9U,A2cMszPuH7OJNOF,tETfPBP
 ODvC,76,76,78J76Je9,7J9PPV9P,763M1PM0d8OHvQ,NPT9Nf9P996VfQTfRH9N9vQ,NM,dEN
 PM5vO3uPl965vPRfNgimBgoBhjphUAgk3imxhkJidBgohgUogjJiUQgjhhkBgmBhixgUYirxhU
 YiZ3jo3YYxhXlmUBAV7ZiUYidZhZRiUYichgUQiXJiZhgi3YqhgmZidRgVZhgBjUAgiZgUYich
 gi3YdZiUAhipijRhZRiUwY1xhh3CKKEGpmWLuCrHum4aEeWqm0GuSrrYgjRAP10GuyKE8qm0ml
 yqqUY3YVJiZ3Ynhik3ijRiZZgUYij322imdRggY3YVRiUQgjhhhBgiZgUYhdphZ3YVJibhihhA
 gV7wichgi3YnZiVJioBhixgUIcgBAELqk2uorGrm0GuWqkGLEGquKKEGrr0GVm4kHCLECLu44f
 9Q,tQdfQ19R9vNn99,7RFHECrmGru0LEqquCLu0GlKKEGqmmqkarmGKLqk4aEeAB.Ahn3Ydpho
 hgiZgZZgUYC84fvQ99N,NOR965vPRfOffP59RHvPRnvaKuWKE4KEOqomqm0GmaKnOKECrl8ro0
 Lu0mtKrlWKE4qtgV7YgdpgahZjZgXpZeRiUogjJCduPZ9RTPObPNbeJRe9PM0PM0T8H9867PMd
 PM,tMTfPhPNZ9R9fQb961fQ9HtKqnaqtGrm8rmGadlr8LmKKt0GuyKEqqkiqm0GuWqmwedphYx
 hrRiUQggBhkJgjBgmZgUwijJifpZBgV7IZdgV7gV7AdB3eDJeI3Y1xhipCUg2Y6xhnZiBhgihi
 nZZUQeZJiqBhXhgnZ37ONhvI9PMZvMF99,tHNPN7OMdPMN76FuPYeZ3D5uPRnRqk4aEeaJcKIE
 Gpomqm4olGroy4v76VuHHeHdOFZ86duH,dI9uETeI786FtI9fQUiAcXZidlI0mWuIW0mRqk2qk
 20JdyoVKIWKJdKIEWGuWqoCLR0GeaKq.EWyqKin4aEVKoXaIbqk2aEYyqtGraUiYedZBhONZ9R
 HvMQbBAV7YcZpiH,umVyqq05in4akWuIW0GWyqRqk4aEc.kdKKuKbeQbBAV7oe,JeUYidZB10I
 bUYed,kRqk282UcIZUwhkZidxhi3Y1xhiNNgAumoqKsyKtGrUmKq0GtKqtKLqGrt0mouKEKrta
 KrSKEGLoaaUQAR166BvPZHnaKqQCjPOdnrKLu0mk002YoBjkhgUAgn3YrhggZhi2YeAZBAV7Qc
 jphq,CbGhgbtQ6837FT1ELaoq0rr6FTvMUWYZUIY2xBUnNFl9QTfQd1UWUWYZUIYjZgXJYg2Yv
 Rcjphq,MOU,lyamRqk2aEIcQ9vNC50mbmoW0022.sC,dQ9PQfPOZPN7HnyKtYichA...JN8PUD
 ZhZZ6oZGhgb,WWDZhZZ6MGP9QMGRfNTf6N763tHN1MFl9Q.U0ZhVRA66HePB1I2irbmqmG2Ahi
 pAvP8rN1akb.I8QC..H0odVZidpiZJYg2YWw7.90..30U2BgoBgWYZUc6MTHtCA,.RdIsQUWY3
 37GTvQd9J.umWWLsEW8poCKoGpmWbWYZUcIHvMF96dONlHEOor8rq4Ku8GK0WIhgsdJsQUWYZU
 QDk2..C5sHQcgBhZphopZ7hhkV7VWEFy2.KIw05.EleqmCKu0GWKqtCKta4TfQ379I2irb.ABg
 DM0.I8QCEFc6T8P..IYgkWqKlKKmGqmGKEyIleqmc6N76y2.OpoK4QD.CLuyaVxgZhjdQbBM0.
 I8Dnt.T0.H0.sI9fQhPNZf998S.g6YgUQejhimRgZJYg223Vv,kt..H.UGtNC5.50..EFK2GKm
 K4.3,3Vv,kt..H.UGtNC56GT1sEkYqKsEWUIVW.IYGVU.,,.G3O3C5c6MTk2..C5.U1,.KJraq
 lyKmKKFmGE27.U3,..KJbaoVyIWKIeKIgGJFm0d0h0b1I2.a..wAb1.kVU7,YeZ3D3,F0kVU3,
 .3,dOFl8JUZFfkt.3,U7..QC.2edRgohimhgnpZ7hBEcaqlI227.f1KIw.EFkaKIe4IXaIaKIc
 aoVGJFm0F0kuUKBB.k2..C5.8ooGrq4KsCLLa2EVaKuEFmGE..bnWW5.c6cEH8JPOEVe6N76W2
 .CbKBB.k2..C5..MG.V01uQ3OOdPPI2I2g7..27.QCh0.M0..b1I2w7G2g6EcaqlgC8GKMH..c
 6QDk2W0dPO2YrBhiZA0mqKrt.7PNUUIZdUChcL3ZoBhgNEUdQbUgV7AVHhgmNOEWyKa4KuKacY
 idZB.N76bON.uqrSrIin4akWkdKKusCPM13OFD09eH7mVyaaFLqE,5TeKKwQ6GLtyKqmqm8rtu
 mdGb1ZimZh2hgn7.Xbs,.oZ1xhiZCU2hgnRg.sEYiV,.Z1...bf9.EWE.8T0E.E8E.k22.0..4
 E,5TeEdKLqKKtCLLC3ZORNX2V.AyI,ktuGdKLqKa2V.Iy1E.0E.ses,0UX8.0E.Qklb8.CLLC3
 bmwmqmGomCb.AS.c9Ajg,0EtT.2.U6UOV.2.86.c918RUohA7ONCK.Y.2..EGE.4E.E.EECOhU
 .wcNC.zwPA.A.2U.E,9z4U...p.0.4.I3,DX1.0E65.2..N6yY,YZPS9L6y0I,5TW.QC5uP..A
 06..E2E.U76.2kLRC,t75J.nT32kwL,,sKFHEfGA3.vn....
 --- end of encoding ---##<=

2. An alternative is to use the WinMerge program (http://winmerge.org). Since these files are in the binary form, there are some "scribbles" in the diffs, but the code and the differences are laid out nicely in the text form.

3. Online view of Subversion repository via ViewVC can also be used to see the .odc files - use "as text" link to see the content of files.
