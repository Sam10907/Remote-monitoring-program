# Remote-monitoring-program
此程式透過反向連接的方式來達成監控客戶端的目的，也就是說由客戶端主動向伺服器發起連線請求，一旦連線成功後就可以開始進行各種監控，這裡我實作了三種功能：

1.遠端操作客戶端的終端機：
  在本機端輸入終端機指令操作遠端的電腦，存取遠端電腦的各種資訊。如圖所示:!(https://github.com/Sam10907/Remote-monitoring-program/blob/main/command.png?raw=true)
  
2.相機監控：
  透過opencv打開電腦的相機鏡頭，然後將錄影的視頻傳到本機端，如此一來就可以看到對方正在做甚麼，由於opencv是第三方套件必須另外安裝，且客戶端不一定有opencv的套件，於是我將這個相機監控的函式另外製作成dll檔且也將有關opencv的dll檔放在同一個目錄下，如此一來就可以做動態調用，而不需在客戶端另外安裝opencv套件庫。
  
3.螢幕監控：
  可以將對方的電腦螢幕畫面傳到本機端，就可知道對方操作電腦的情形，實作方式透過windows api和opencv套件庫來完成。

運作方式：
首先將shellexe.cpp編譯成install.exe，這個執行檔會將所需的檔案移到事先創建在系統碟的資料夾裡，然後更改windows的註冊表使其開機自啟動監控程式，此程式只能運作在windows的環境，並且使用visual studio 2010 編寫而成。
