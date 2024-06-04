//Final code
//----------------------------------------------------------
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
unsigned long count = 0;
unsigned long count1 = 0;
double sumX[50];
double sumY[50];
double sumZ[50];
double AvgX = 0;
double AvgY = 0;
double AvgZ = 0;
double accelerometerY[10];
double accelerometerZ[10];
double gyroscopeX[100];
double accelerometerDataX[100];
double accelerometerDataY[100];
double accelerometerDataZ[100];
double GX = 0;
double AY = 0;
double AZ = 0;
double tthres = 70;
double gthres = 0.05;
int WindowL = 45;
int WindowR = 80;
double windowl[45][3];
double windowr[80][3];
int prev =0;
double result[2];
double final[125][3];

void setup() {
  Serial.begin(115200);
  Wire.begin(5,4);
  if (!mpu.begin()) {
    Serial.println("Failed to initialize MPU6050!");
    while (1);
  } else {
    Serial.println("Success");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  for (int count = 0; count < 100; count++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    // Store accelerometer data in arrays
    accelerometerDataX[count] = g.gyro.x;
    accelerometerDataY[count] = a.acceleration.y;
    accelerometerDataZ[count] = a.acceleration.z;

    if (count >= 50) {
      // Calculate the average using the stored data

      for (int i = count - 50; i < count; i++) {
        sumX[count - 50] += accelerometerDataX[i];
        sumY[count - 50] += accelerometerDataY[i];
        sumZ[count - 50] += accelerometerDataZ[i];
      }
      sumX[count - 50] /= 50.0;
      sumY[count - 50] /= 50.0;
      sumZ[count - 50] /= 50.0;
    }

    if (count == 99) {
      // Calculate the final average after collecting 100 data points

      for (int i = 0; i < 50; i++) {
        AvgX += sumX[i];
        AvgY += sumY[i];
        AvgZ += sumZ[i];
      }
      AvgX /= 50.0;
      AvgY /= 50.0;
      AvgZ /= 50.0;

    }
    delay(20);
  }
  Serial.println("Caliberated");
}

void loop() {
  sensors_event_t a, g,temp;
  mpu.getEvent(&a, &g,&temp);
  accelerometerY[count1 % 10] = a.acceleration.y;
  accelerometerZ[count1 % 10] = a.acceleration.z;
  gyroscopeX[count1%10]=g.gyro.x;

  if (count1 >= 9) {
    // Calculate the average of the last 10 readings
    double sumGX = 0.0;
    double sumAY = 0.0;
    double sumAZ = 0.0;
    for (int i = 0; i < 10; i++) {
      sumGX += gyroscopeX[i];
      sumAY += accelerometerY[i];
      sumAZ += accelerometerZ[i];
    }
    double avgGX = sumGX / 10.0;
    double avgAY = sumAY / 10.0;
    double avgAZ = sumAZ / 10.0;

    // Calculate AY and AZ based on the average and sumX, sumZ
    GX = (avgGX - AvgX);
    AY = (avgAY - AvgY);
    AZ = (avgAZ - AvgZ);

    windowl[count1%WindowL][2] = GX;
    windowl[count1%WindowL][0] = AY;
    windowl[count1%WindowL][1] = AZ;
}
delay(17);
count1++;

if (GX >= gthres && (count1 - prev)>=tthres)
{
  prev = count1;
  for(int v=0; v < WindowR; v++)
  {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  accelerometerY[count1 % 10] = a.acceleration.y;
  accelerometerZ[count1 % 10] = a.acceleration.z;
  gyroscopeX[count1%10]=g.gyro.x;

    // Calculate the average of the last 10 readings
    double sumGX = 0.0;
    double sumAY = 0.0;
    double sumAZ = 0.0;
    for (int i = 0; i < 10; i++) {
      sumGX += gyroscopeX[i];
      sumAY += accelerometerY[i];
      sumAZ += accelerometerZ[i];
    }
    double avgGX = sumGX / 10.0;
    double avgAY = sumAY / 10.0;
    double avgAZ = sumAZ / 10.0;

    // Calculate AY and AZ based on the average and sumX, sumZ
    GX = (avgGX - AvgX);
    AY = (avgAY - AvgY);
    AZ = (avgAZ - AvgZ); 
    windowr[v][2] = GX;
    windowr[v][0] = AY;
    windowr[v][1] = AZ;
    delay(17);
    count1++;
  }
  
  int tempcount = count1 - WindowR;
  int w;
  for(int v=0; v < (WindowL) ; v++)
  {
    w= v + (tempcount + 1);
    final[v][0]=windowl[w%WindowL][0];
    final[v][1]=windowl[w%WindowL][1];
    final[v][2]=windowl[w%WindowL][2];
  }
  for(int v=0; v < WindowR; v++)
  {
    w=v + WindowL;
    final[w][0]=windowr[v][0];
    final[w][1]=windowr[v][1];
    final[w][2]=windowr[v][2];
  } 
  w=0;
  for (int v=WindowR - (WindowL - 1); v < WindowR; v++)
  {    
    windowl[(count1+w)%WindowL][0]=windowr[v][0];
    windowl[(count1+w)%WindowL][1]=windowr[v][1];
    windowl[(count1+w)%WindowL][2]=windowr[v][2];
    w++;
  }

  double finall[375];
  for(int i=0;i<125;i++){
    finall[i*3]=final[i][0];
    finall[i*3 +1]=final[i][1];
    finall[i*3 +2]=final[i][2];
}
score(finall,result);
if (result[0]>=0.5){
  Serial.println("Step");
}
else{
  Serial.println("Not Step");
}
}
}

void score(double *input, double *output) {
    double var0[2];
    
    if (input[93] <= -0.5979099869728088) {
        if (input[12] <= -0.6660099923610687) {
            if (input[26] <= 0.0189800001680851) {
                double temp[] = {0.0, 1.0};
                memcpy(var0, temp, 2 * sizeof(double));
            } else {
                double temp[] = {0.3333333333333333, 0.6666666666666666};
                memcpy(var0, temp, 2 * sizeof(double));
            }
        } else {
            if (input[161] <= -0.05234000086784363) {
                if (input[270] <= 0.4892899990081787) {
                    if (input[101] <= 0.16079000383615494) {
                        double temp[] = {1.0, 0.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        double temp[] = {0.4, 0.6};
                        memcpy(var0, temp, 2 * sizeof(double));
                    }
                } else {
                    if (input[198] <= 3.695340096950531) {
                        double temp[] = {0.3333333333333333, 0.6666666666666666};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    }
                }
            } else {
                if (input[98] <= -0.02171000000089407) {
                    if (input[153] <= 0.47082000970840454) {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        if (input[282] <= 0.16729000210762024) {
                            double temp[] = {0.6, 0.4};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            double temp[] = {0.0, 1.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[8] <= -0.038690000772476196) {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        double temp[] = {1.0, 0.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    }
                }
            }
        }
    } else {
        if (input[272] <= -0.05024999938905239) {
            if (input[363] <= 0.39229001104831696) {
                double temp[] = {1.0, 0.0};
                memcpy(var0, temp, 2 * sizeof(double));
            } else {
                if (input[140] <= -0.04725000075995922) {
                    double temp[] = {1.0, 0.0};
                    memcpy(var0, temp, 2 * sizeof(double));
                } else {
                    if (input[19] <= -0.2413799986243248) {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        double temp[] = {1.0, 0.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[90] <= 0.38352999091148376) {
                if (input[198] <= -0.6310099959373474) {
                    if (input[262] <= -0.322380006313324) {
                        if (input[125] <= -0.026209999807178974) {
                            double temp[] = {0.4, 0.6};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            double temp[] = {1.0, 0.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        }
                    } else {
                        if (input[265] <= 0.0867800023406744) {
                            double temp[] = {0.0, 1.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            if (input[261] <= -0.19200999289751053) {
                                double temp[] = {0.16666666666666666, 0.8333333333333334};
                                memcpy(var0, temp, 2 * sizeof(double));
                            } else {
                                double temp[] = {1.0, 0.0};
                                memcpy(var0, temp, 2 * sizeof(double));
                            }
                        }
                    }
                } else {
                    if (input[90] <= -0.1990099996328354) {
                        if (input[355] <= -0.3948799967765808) {
                            double temp[] = {1.0, 0.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            if (input[346] <= 0.09627999924123287) {
                                double temp[] = {0.0, 1.0};
                                memcpy(var0, temp, 2 * sizeof(double));
                            } else {
                                double temp[] = {0.9090909090909091, 0.09090909090909091};
                                memcpy(var0, temp, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[135] <= 0.3538200110197067) {
                            if (input[89] <= -0.0003499999875202775) {
                                double temp[] = {1.0, 0.0};
                                memcpy(var0, temp, 2 * sizeof(double));
                            } else {
                                double temp[] = {0.6666666666666666, 0.3333333333333333};
                                memcpy(var0, temp, 2 * sizeof(double));
                            }
                        } else {
                            if (input[250] <= -0.06385999917984009) {
                                double temp[] = {1.0, 0.0};
                                memcpy(var0, temp, 2 * sizeof(double));
                            } else {
                                double temp[] = {0.3, 0.7};
                                memcpy(var0, temp, 2 * sizeof(double));
                            }
                        }
                    }
                }
            } else {
                if (input[12] <= -0.3204699903726578) {
                    if (input[146] <= -0.02171000000089407) {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        if (input[334] <= -0.21250999718904495) {
                            double temp[] = {1.0, 0.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            double temp[] = {0.6666666666666666, 0.3333333333333333};
                            memcpy(var0, temp, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[301] <= 0.2469800040125847) {
                        double temp[] = {0.0, 1.0};
                        memcpy(var0, temp, 2 * sizeof(double));
                    } else {
                        if (input[88] <= -0.11822999641299248) {
                            double temp[] = {0.0, 1.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        } else {
                            double temp[] = {1.0, 0.0};
                            memcpy(var0, temp, 2 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    memcpy(output, var0, 2 * sizeof(double));
}
//----------------------------------------------------------
