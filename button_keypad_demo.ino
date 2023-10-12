#include <crt_CleanRTOS.h>        // This file includes crt_Config.h  You'll need to change defines there for a release build.
#include "crt_input.h"

namespace crt
{
	//instances of this class handle buttonevents
	class MyButtonListener : public Task, public ButtonListener
	{
		private:
		Queue<button_id_t, 5> buttonPress_Queue;
		Queue<button_id_t, 5> buttonRelease_Queue;	

	public:
		MyButtonListener(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber) :
			Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber), 
			buttonPress_Queue(this), buttonRelease_Queue(this)
		{
			start(); // For simplicity, the task is started right away in it's constructor.
		}
		
		//override this two methods from ButtonListener
		void buttonPressed(button_id_t button_id){
			buttonPress_Queue.write(button_id);
		}
		
		void buttonReleased(button_id_t button_id){
			buttonRelease_Queue.write(button_id);
		}

	private:
		void main()
		{
			vTaskDelay(1000); // wait for other threads to have started up as well.
			button_id_t id;

			while (true)
			{
				waitAny(buttonPress_Queue + buttonRelease_Queue);
				if(hasFired(buttonPress_Queue)){
					buttonPress_Queue.read(id);
					ESP_LOGI(taskName, "buttonpressed %d", id);
				}
				else if(hasFired(buttonRelease_Queue)){
					buttonRelease_Queue.read(id);
					ESP_LOGI(taskName, "buttonreleased %d", id);
				}		
				vTaskDelay(1);
			}
		}
	}; // end class ButtonMyListener
	
	//instances of this class handle keyevents from Keypad
	class MyKeyListener : public Task, public KeyListener {
		private:
		Queue<char, 5> keyPress_Queue;
		
		public:
		MyKeyListener(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber) :
			Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber), 
			keyPress_Queue(this)
		{
			start(); // For simplicity, the task is started right away in it's constructor.
		}
		
		//override this method from KeyListener
		void keyPressed(char ch){
			keyPress_Queue.write(ch);
		}
		
		private:
		void main()
		{
			vTaskDelay(1000); // wait for other threads to have started up as well.
			char ch;

			while (true)
			{
				wait(keyPress_Queue);
				if(hasFired(keyPress_Queue)){
					keyPress_Queue.read(ch);
					ESP_LOGI(taskName, "keypressed %c", ch);
				}					
				vTaskDelay(1);
			}
		}
	};//class mykeylistener
	
    MainInits mainInits;            // Initialize CleanRTOS
	Button button( 22, BLUE);      // Button connected to pin 22 and ground, we use INPUT_PULLUP
	MyButtonListener  buttonlistenerTask   ("ButtonListenTask"   , 2 /*priority*/, 4000 /*stackBytes*/, ARDUINO_RUNNING_CORE);
	int row_pins[4] = {19, 18, 5, 17};
	int col_pins[4] = { 16, 4, 0, 2};
	KeyPad keypad(row_pins, col_pins); //Keypad connected to 8 pins
	MyKeyListener  keylistenerTask   ("KeyListenTask"   , 2 /*priority*/, 4000 /*stackBytes*/, ARDUINO_RUNNING_CORE);
	Handler theHandler("HandlerTask", 1, 4000, ARDUINO_RUNNING_CORE); //active object that activates button and keypad every 200 ms.
}

void setup()
{
	vTaskDelay(10);// allow logger to initialize.
	ESP_LOGI("checkpoint", "start of main");
	vTaskDelay(1);
	crt::button.addListener(&crt::buttonlistenerTask);
	crt::keypad.addListener(&crt::keylistenerTask);
	crt::theHandler.addUpdatable(&crt::button);
	crt::theHandler.addUpdatable(&crt::keypad);
	
}

void loop()
{
	vTaskDelay(1);// Nothing to do in loop - all example code runs in the threads above. That is theHandler, keyListenerTask and buttonListenerTask.
}
