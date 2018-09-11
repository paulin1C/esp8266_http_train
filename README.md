# esp8266_http_train
Use a wemos with motor shield to remote controll a old model train via http.
## Parts needed
* old model train
* WEMOS D1 mini
* WEMOS D1 mini motor shield
* Step-down converter from 12V (or watever your rail voltage to 3.3V or 5V)
## Wireing
* Plug the motor shield into the wemos an make shure that the STBY jumper on the motor shield is set to I2C.
* Cut the connection between wheels and the motor.
* Connect the motor to A1 and A2.
* Wire VM and GND to the wheels.
* Find a way to supply 3.3V to the wemos, for example with a buck converter.
You might want to use a recivier to prevent the components to be fried when the train is put on the rails the wrong way round ;)
