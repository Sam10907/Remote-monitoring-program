# Remote-monitoring-program
此程式透過反向連接的方式來達成監控客戶端的目的，也就是說由客戶端主動向伺服器發起連線請求，一旦連線成功後就可以開始進行各種監控，這裡我實作了三種功能：

1.遠端操作客戶端的終端機：
  在本機端輸入終端機指令操作遠端的電腦，存取遠端電腦的各種資訊。
2.相機監控：
  透過opencv打開電腦的相機鏡頭，然後將錄影的視頻傳到本機端，如此一來就可以看到對方正在做甚麼。
3.螢幕監控：
  可以將對方的電腦螢幕畫面傳到本機端，就可知道對方操作電腦的情形。
