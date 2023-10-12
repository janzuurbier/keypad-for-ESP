#pragma once
#include <crt_CleanRTOS.h>
#include <Arduino.h>
// J. Zuurbier 2023
// Bevat klassen Button en Keypad voor een ESP32.
// Bevat tevens hulpklasse Handler en interfaces KeyListener en ButtonListener

namespace crt
{
	const int MAX_NUM_LISTENERS = 5;
	const int MAX_NUM_UPDATABLES = 8;
	const int NUM_ROWS = 4;
	const int NUM_COLS = 4;
	enum button_id_t {RED, YELLOW, BLUE, GREEN, WHITE};
	
	class Updatable{
	public:
		virtual void update() = 0;
	};
	
	//Een object van klasse Handler activeert Buttons en Keypads elke 200 ms.
	class Handler : public Task
	{
	private:
		Updatable* updatables[MAX_NUM_UPDATABLES];
		int num_updatables = 0;
		
		void main()
		{
			vTaskDelay(1000);
			while(true){
				for(int i = 0; i < num_updatables; i++)
					updatables[i]->update();
				vTaskDelay(200);
			}
		}
		
		public:
			Handler(const char *taskName, unsigned int taskPriority, unsigned int taskSizeBytes, unsigned int taskCoreNumber): 
			   Task(taskName, taskPriority, taskSizeBytes, taskCoreNumber)
			{
				start();
			}
			
			void addUpdatable(Updatable* u){
				if(num_updatables < MAX_NUM_UPDATABLES){ 
					updatables[num_updatables] = u;
					num_updatables++;
				}
			}	
	};
	
	class ButtonListener{
	public: 
		virtual void buttonPressed(button_id_t b) = 0;
		virtual void buttonReleased(button_id_t b ) = 0;
	};
	
	
	class Button : public Updatable
	{
	private:
		int numListeners = 0;
		bool isPressed = false;
		ButtonListener* listeners[MAX_NUM_LISTENERS];
		const uint8_t PIN;
		button_id_t id;
		bool repeats_allowed;

	public:
		Button(uint8_t pin, button_id_t id, bool repeats_allowed = false) : 	PIN(pin), id(id), repeats_allowed(repeats_allowed)
		{
			pinMode(PIN, INPUT_PULLUP);
		}
		
		void addListener(ButtonListener* bl){
			if (numListeners < MAX_NUM_LISTENERS){
				listeners[numListeners] = bl;
				numListeners++;
			}	
		}

		void update()
		{
			int val = digitalRead(PIN);
			if (val == LOW && (!isPressed || repeats_allowed )){
				isPressed = true;
				for(int i = 0; i < numListeners; i++) listeners[i]->buttonPressed(id);
			}
			if (val == HIGH && isPressed) {
				isPressed = false;
				for(int i = 0; i < numListeners; i++) listeners[i]->buttonReleased(id);
			}
		}
	}; // end class Button
	
	class KeyListener
	{
		public:
		virtual void keyPressed(char key) = 0;
	};
	
	class KeyPad : public Updatable{
		private:
		
		const char keys[NUM_ROWS][NUM_COLS] = { {'1', '2', '3', 'A'}, {'4', '5', '6', 'B'} , {'7', '8', '9', 'C'} , {'*', '0', '#', 'D'} } ;
		
		int row_pins[NUM_ROWS];
		int col_pins[NUM_COLS];
		
		KeyListener* theListeners[MAX_NUM_LISTENERS];
		int numListeners = 0;
		bool repeats_allowed;
		
		
		public :
		
		KeyPad(int r[4], int  c[4], bool repeats_allowed = false): repeats_allowed(repeats_allowed){
			for(int i = 0; i < NUM_ROWS; i++) row_pins[i] = r[i];
			for(int i = 0; i < NUM_COLS; i++) col_pins[i] = c[i];
			for(int i = 0; i < NUM_ROWS; i++) pinMode(row_pins[i], OUTPUT);
			for(int i = 0; i < NUM_COLS; i++) pinMode(col_pins[i], INPUT_PULLUP);
			for(int i = 0; i < NUM_ROWS; i++) digitalWrite(row_pins[i], LOW);
		}
		
		void addListener(KeyListener* kl){
			if (numListeners < MAX_NUM_LISTENERS){
				theListeners[numListeners] = kl;
				numListeners++;
			}	
		}
		
		bool has_been_processed = false;
		void update(){	 
			int col = -1, row = -1;
			bool keypressed = false;
			//scan keys
			for(int i = 0; i < NUM_COLS; i++){
				if(digitalRead(col_pins[i]) == LOW){
					col = i;
					keypressed = true;
				}
			}
			if (!keypressed) 
				has_been_processed = false; //next keypress must be processed!
			else if(!has_been_processed || repeats_allowed) {
				for(int i = 0; i < NUM_ROWS; i++) {
					digitalWrite(row_pins[i], HIGH);
					vTaskDelay(1);
					if(digitalRead(col_pins[col]) == HIGH)
						row = i;
					digitalWrite(row_pins[i], LOW);
				}
			//notify listeners
				if (row >= 0 && col >= 0 && row < NUM_ROWS && col < NUM_COLS ){
					for(int i = 0; i < numListeners; i++) {
						theListeners[i]->keyPressed(keys[row][col]);
					}
				}
				has_been_processed = true;
			}				
		}
	};// end class keypad
};// end namespace crt