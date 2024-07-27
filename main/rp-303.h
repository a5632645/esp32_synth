#include <math.h>

#define SAMPLE_RATE 48000
const float TO_RAD = 2.0 * M_PI / SAMPLE_RATE;
const float TO_256 = 256.0 / SAMPLE_RATE;
#include "HiiroFoxDSP.h"
#include "my_adc.h"

// class ScanKB {  //键盘类
// private:
//   int wp[4] = { 17, 18, 19, 20 };
//   int hp[4] = { 13, 14, 15, 16 };
//   int result[16];
// public:
//   ScanKB() {
//     for (int i = 0; i < 4; ++i) {
//       pinMode(wp[i], OUTPUT);
//       pinMode(hp[i], INPUT);
//       digitalWrite(wp[i], 0);
//     }
//   }
//   void scan() {
//     for (int i = 0; i < 4; ++i) {
//       digitalWrite(wp[i], 1);
//       for (int j = 0; j < 4; ++j) {
//         if (result[i * 4 + j] != -1)
//           result[i * 4 + j] = digitalRead(hp[j]);
//         else if (digitalRead(hp[j]) == 0)
//           result[i * 4 + j] = 0;
//       }
//       digitalWrite(wp[i], 0);
//     }
//   }
//   int KeyState(int n) {
//     return result[n] != 0;
//   }
//   int KeyStateOnce(int n) {
//     int a = result[n];
//     if (a == 1) result[n] = -1;
//     return a;
//   }
// };
class RP303 {
private:
//   ScanKB skb;

  //float Vsyncfreq2 = 0;
  float Vdecay = 0;  //adc调参
  float Vbpm = 0;
  float Voct = 0;
  float Vdrive = 0;  //失真
  float Vdist = 0;
  float Vpost = 0;
  float Vctof = 0;  //主滤波器
  float Vreso = 0;
  float VLPCtof = 0;  //失真后的lp
  float VHPCtof = 0;  //失真前的hp
  float Vslide = 0;
  float Vkbcf = 0;

  Filter adcfilt1, adcfilt2, adcfilt3, adcfilt4;     //adc的滤波器
  Filter adcfilt5, adcfilt6, adcfilt7, adcfilt8;     //adc的滤波器
  Filter adcfilt9, adcfilt10, adcfilt11, adcfilt12;  //adc的滤波器


  float loopfreq[128] = { 0 };  //音序器信息
  char loopgate[128] = { 0 };
  float SawFreq = 55, CutOff = 0, KbcFreq = 1.0;
  int isPlay = 0;
  int looplen = 0, looppos = 0, lastup = 0;

  float t = 0;  //音符更新

  Filter filt1;
  Filter filt2;
  Filter filt3;
  Filter filt4;
  Filter filt5;
  Filter filt6;

  VCO vco1;
  RP808DRUM drum1;
  HF_Sine lop1, lop2;
  HF_Sine rop1, rop2;
  phaser eff_phs;
  HF_Delay ldly, rdly;

public:
    void Init() {
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_0);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_1);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_2);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_3);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_4);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_5);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_6);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_7);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_8);
        SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_9); // 10
        SimpleAdcInit(ADC_UNIT_2, ADC_CHANNEL_0);
        SimpleAdcInit(ADC_UNIT_2, ADC_CHANNEL_1); // 12
    }

  void UpdataAlgm() {
    vco1.AutoZero();
    filt1.AutoZero();
    filt2.AutoZero();
    filt3.AutoZero();
    filt4.AutoZero();
    filt5.AutoZero();
    filt6.AutoZero();
  }
  void UpdataADC() {
    float tmpv1 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_0);
    float tmpv2 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_1);
    float tmpv3 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_2);
    float tmpv4 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_3);
    float tmpv5 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_4);
    float tmpv6 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_5);
    float tmpv7 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_6);
    float tmpv8 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_7);
    float tmpv9 =SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_8);
    float tmpv10 = SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_9);
    float tmpv11 = SimpleAdcRead(ADC_UNIT_2, ADC_CHANNEL_0);
    float tmpv12 = SimpleAdcRead(ADC_UNIT_2, ADC_CHANNEL_1);
    //vco
    Vdrive = adcfilt1.LPF1(tmpv1, 0.3, 0) / 4095.0 * 32.0 + 1.0;
    Vdist = 1.0 - adcfilt2.LPF2(tmpv2, 0.3, 0) / 4095.0 * 1.0;
    Vpost = adcfilt3.LPF1(tmpv3, 0.3, 0) / 4095.0 * 4.0;

    //setting
    Vbpm = (adcfilt4.LPF1(tmpv4, 0.1, 0) / 4095.0 + 0.2) * 200.0 / SAMPLE_RATE / 12.0;
    Vdecay = 1.0 - pow(adcfilt5.LPF1(tmpv5, 0.3, 0) / 4095.0, 3.0) * 0.002;
    Voct = pow(2, (int)(adcfilt6.LPF1(tmpv6, 0.3, 0) / 4095.0 * 8.0));

    //base filter
    Vctof = adcfilt7.LPF1(tmpv7, 0.3, 0) / 4095.0;  //1.85
    Vctof = Vctof * Vctof * 1.95;
    Vreso = sqrt(sqrt(adcfilt8.LPF1(tmpv8, 0.3, 0) / 4095.0)) * 0.95;

    //Other filter
    VHPCtof = adcfilt9.LPF1(tmpv9, 0.3, 0) / 4095.0;
    VLPCtof = adcfilt10.LPF1(tmpv10, 0.3, 0) / 4095.0;
    VHPCtof = VHPCtof * VHPCtof;
    VLPCtof = VLPCtof * VLPCtof;

    Vslide = adcfilt11.LPF1(tmpv11, 0.3, 0) / 4095.0;
    Vslide = Vslide * Vslide * Vslide * Vslide * Vslide;
    Vkbcf = 1.0 - adcfilt12.LPF1(tmpv12, 0.3, 0) / 4095.0;


    //oct
  }

  void NoteOn(int note, float velocity) {
    SawFreq = 110.0f * powf(2.0f, (note - 69.0f) / 12.0f);
    KbcFreq = SawFreq / 110.0f * Voct;
    CutOff = 1;
  }

  void NoteOff(int note, float velocity) {

  }
//   void UpdataKB() {                                                            //不用看，这就是屎山。我没想好很好的方法来写，因为扫描键盘毕竟和普通键盘不一样。
//     // skb.scan();                                                                //更新键盘
//     //if (skb.KeyState(0) && skb.KeyState(1) && skb.KeyState(2)) esp_restart();  //软复位
//     if (skb.KeyState(13) && skb.KeyState(15)) {                                //清空音序器
//       memset(loopfreq, 0, sizeof(loopfreq));
//       memset(loopgate, 0, sizeof(loopgate));
//       looplen = 0;  //清空音序器
//       looppos = 0;
//       lastup = 0;
//       isPlay = 0;
//     }
//     if (skb.KeyStateOnce(14) == 1) {  //模式选择
//       isPlay = !isPlay;
//       if (isPlay == 0) {
//         looppos = 0;  //重置位置
//       }
//     }
//     if (isPlay == 0) {
//       int thisFreq = -999999, isdown = 0;
//       for (int i = 0; i < 13; ++i) {  //检测按下
//         if (skb.KeyStateOnce(i) == 1) isdown = 1, thisFreq = i;
//       }
//       if (skb.KeyStateOnce(13) == 1) {
//         SawFreq = 0;
//         KbcFreq = 0;
//         loopgate[looplen] = 0;
//       } else if (isdown == 1) {  //存在按下
//         SawFreq = 110.0 * pow(2.0, (float)(thisFreq - 36 + 3) / 12.0);
//         KbcFreq = SawFreq / 110.0 * Voct;
//         CutOff = 1;
//         loopgate[looplen] = 1;
//       }
//       if (skb.KeyStateOnce(15) == 1) {           //记录
//         loopfreq[looplen] = SawFreq * Voct / 4;  //音序 freq
//         looplen++;
//       }
//     } else {  //升调模式
//       int thisFreq = -999999, isdown = 0;
//       for (int i = 0; i < 13; ++i) {  //检测按下
//         if (skb.KeyStateOnce(i) == 1) isdown = 1, thisFreq = i;
//       }
//       if (isdown == 1) {
//         for (int i = 0; i < looplen; ++i) {  //统一升调
//           loopfreq[i] *= pow(2.0, (float)(thisFreq - lastup) / 12.0);
//         }
//         lastup = thisFreq;
//       }
//     }
//   }
  void proc(float* data, int len) {
    for (int i = 0; i < len; i += 2) {
      float datL = 0.0, datR = 0.0;

      int sign = vco1.Saw_NoUnison(filt1.LPF1_NoReso(SawFreq * Voct, Vslide));
      float now_ctof = (CutOff *= Vdecay) * Vctof * ((1.0 - Vkbcf) * KbcFreq + Vkbcf);
      now_ctof = now_ctof > 1.95 ? 1.95 : now_ctof;
      sign = filt2.LPF2_Oversampling_ResoLimit_limit(sign, now_ctof, Vreso);
      sign = filt3.LPF2_Oversampling_ResoLimit_limit(sign, now_ctof, Vreso);
      sign = filt5.HPF1_NoReso(sign, VHPCtof);
      sign = Vpost * Dist_Limit_Float(sign * Vdrive, Vdist, SAMPLE_RATE >> 1);
      sign = filt6.LPF1_NoReso(sign, VLPCtof);

      datL = sign;
      datR = sign;

      data[i + 0] = datL;
      data[i + 1] = datR;
    }
  }
};