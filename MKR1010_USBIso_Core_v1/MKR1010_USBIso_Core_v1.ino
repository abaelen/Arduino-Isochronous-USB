#include "CDCIso.h"
#include "CDCIso_event.h"


void setup() {
  // put your setup code here, to run once:
  USBISO.iso_pcksize=1024;
  USB_SetHandler(UDD_Handler_Iso);
  SerialUSB.print("test");
  delay(1000);
  SerialUSB.print("test");
  delay(1000);

  
    for (int i = 0; i < sizeof(USBISO.ADC_Bfr)/sizeof(USBISO.ADC_Bfr[0]); i++)
    {
        USBISO.ADC_Bfr[i] = 0;
    }

    uint32_t c = 1023*8-1; //bits
    uint32_t t = 8191;

   // for (int i = 0; i < (uint32_t) (1022*8)/(USBISO.ADC_Reading_Size+USBISO.ADC_Tick_Size+USBISO.ADC_Chk_Size); i++)
     for (int i = 0; i < (uint32_t) (1022*8)/(USBISO.ADC_Reading_Size+USBISO.ADC_Tick_Size+USBISO.ADC_Chk_Size); i++)
    {
        (USBISO.TSmpl <= 0) ? USBISO.TSmpl = 8191 : USBISO.TSmpl--;
        (USBISO.CSmpl >= 4000)? USBISO.CSmpl = 0 : USBISO.CSmpl++; //CSmpl=CSmpl;
        //USBISO.ADC_Reading = (uint32_t)floor((sin((double)(CSmpl) / (double)4000 * 2.0f * 3.14f) + 1.0f) / 2.0f * pow(2, 8));
        USBISO.ADC_Reading = USBISO.CSmpl;
        USBISO.ADC_Tick = USBISO.TSmpl; (t==0)?t=8191 : t--;

        //c--;
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Tick_Size, (uint64_t*)&USBISO.ADC_Tick);
        c = c - USBISO.ADC_Tick_Size; 
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Reading_Size, (uint64_t*)&USBISO.ADC_Reading);
        c = c - USBISO.ADC_Reading_Size; 
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Chk_Size, (uint64_t *) &USBISO.ADC_Chk);
        c = c - USBISO.ADC_Chk_Size; 
      
    }
    USBISO.lenwritebuffer=sizeof(USBISO.writebuffer);
    USBISO.write(USBISO.writebuffer,1023);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (USBISO.Continue==1) {
    USBISO.Continue=0;
    for (int i = 0; i < sizeof(USBISO.ADC_Bfr)/sizeof(USBISO.ADC_Bfr[0]); i++)
    {
        USBISO.ADC_Bfr[i] = 0;
    }

    uint32_t c = 1023*8-1; //bits
    uint32_t t = 8191;

   // for (int i = 0; i < (uint32_t) (1022*8)/(ADC_Reading_Size+ADC_Tick_Size+ADC_Chk_Size); i++)
     for (int i = 0; i < (uint32_t) (1022*8)/(USBISO.ADC_Reading_Size+USBISO.ADC_Tick_Size+USBISO.ADC_Chk_Size); i++)
    {
        (USBISO.TSmpl <= 0) ? USBISO.TSmpl = 8191 : USBISO.TSmpl--;
        (USBISO.CSmpl >= 4000)? USBISO.CSmpl = 0 : USBISO.CSmpl++; //CSmpl=CSmpl;
        //ADC_Reading = (uint32_t)floor((sin((double)(CSmpl) / (double)4000 * 2.0f * 3.14f) + 1.0f) / 2.0f * pow(2, 8));
        USBISO.ADC_Reading = USBISO.CSmpl;
        USBISO.ADC_Tick = USBISO.TSmpl; (t==0)?t=8191 : t--;

        //c--;
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Tick_Size, (uint64_t*)&USBISO.ADC_Tick);
        c = c - USBISO.ADC_Tick_Size; 
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Reading_Size, (uint64_t*)&USBISO.ADC_Reading);
        c = c - USBISO.ADC_Reading_Size; 
        USBISO.BinConcat(USBISO.ADC_Bfr, c, USBISO.ADC_Chk_Size, (uint64_t *) &USBISO.ADC_Chk);
        c = c - USBISO.ADC_Chk_Size; 
      
    }
    for (int i = 0; i<1023;i++) {
      USBISO.writebuffer[i] =  USBISO.ADC_Bfr[i];
    }
  }
}
