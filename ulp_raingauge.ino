#include "hulp_arduino.h"
#define PIN_PLUV    GPIO_NUM_34
#define ULP_WAKEUP_INTERVAL_MS (75) //Debouncer timer 
int TIME_TO_SLEEP = 10; //Time EPS32 will go to sleep in seconds

RTC_DATA_ATTR ulp_var_t counter_pluv;
float pluviometre = 0;

void ulp_init(){
  enum {
        LBL_PIN_PLUV_TRIGGERED,  //This labels are use as if function
        
  };
  const ulp_insn_t program[] = {
    //ULP
    I_MOVI(R1,0), //Copy an immediate value into register: R1
    I_GPIO_READ(PIN_PLUV),
    //If it's set, goto LAB_PIN_PLUV_TRIGGERED
    M_BGE(LBL_PIN_PLUV_TRIGGERED, 1),
    //Else HALT
      I_HALT(),    //Halt the coprocessor but keeps ULP timer active
    //
    M_LABEL(LBL_PIN_PLUV_TRIGGERED),
      //increment counter if PIN_PLUV TRIGGERED
      I_GET(R0,R1, counter_pluv), //Load value from RTC memory into reg_dest register.
      I_ADDI(R0,R0,1),  //R0++
      I_PUT(R0,R1, counter_pluv), //Load value from reg_dest register to RTC memory.
      M_WAKE_WHEN_READY(),
      I_HALT(),

  };
  //Pin configuration
  hulp_peripherals_on();
  hulp_configure_pin(PIN_PLUV, RTC_GPIO_MODE_INPUT_ONLY, GPIO_PULLDOWN_ONLY, 0);
  ESP_ERROR_CHECK(hulp_configure_pin_int(PIN_PLUV, GPIO_INTR_ANYEDGE));
  ESP_ERROR_CHECK(hulp_ulp_load(program, sizeof(program), 1000UL * ULP_WAKEUP_INTERVAL_MS, 0));
  ESP_ERROR_CHECK(hulp_ulp_run(0));
  
}

void sleep (){
  Serial.println("Going to sleep for "+ String(TIME_TO_SLEEP) +  " seconds");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1000000UL);
  delay(100);
  Serial.flush();
  //esp_sleep_enable_ulp_wakeup(); //This is for debugging
  esp_deep_sleep_start();
}

void Pluviometre(){

  pluviometre = counter_pluv.val * 0.2794;
  counter_pluv.val = 0;

  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Program is starting");
  ulp_init();
  Serial.println("counter = " + String(counter_pluv.val));
  Pluviometre();
  Serial.println("Pluv counter = " + String(pluviometre) + " mm");
  sleep();
  
}

void loop() {
}
