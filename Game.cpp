#include "Game.hpp"


static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
/*void tempprintboard(int** field,int height,int width,int iteration){
    cout << "--------------------------" << iteration << "--------------------------" << endl;
    for(int i=0;i<height;i++){
        for (int j = 0; j < width-1; j++) {
            cout << field[i][j] << " ";
        }
        cout << field[i][width-1] << endl;
    }
}*/
void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(nullptr);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {

	// Create game fields - Consider using utils:read_file, utils::split
	// Create & Start threads
	// Testing of your implementation will presume all threads are started here
	vector<string> file = utils::read_lines(this->filename);
    height = file.size();
    width = ((int)file[0].size()+1)/2;
    if(m_thread_num>=height)
        m_thread_num = height; // TODO MAKE SURE THAT WE HAVE TO HAVE MIN THREADS
    field = new int*[height];
    for (int i = 0; i < height; ++i)
        field[i] = new int[width];
    tempfield = new int*[height]; //temp field serves for phase 1 and phase 2 new calculations
    for (int i = 0; i < height; ++i)
        tempfield[i] = new int[width];
    for(int i=0;i<height;i++){
        vector<string> splitline = utils::split(file[i],' '); //FIXED LAGGY IMPLEMENTATION
        for (int j = 0; j < width; j++) {
            field[i][j] = std::stoi(splitline[j],0,10);
        }
    }
    //finished board initialization
    //begin thread initialization
    for (int i = 0; i < m_thread_num; ++i) {
        m_threadpool.push_back(new WorkerThread(i,this->task_queue,&this->m_tile_hist,&this->mutex,this->barrier));
        m_threadpool[i]->start();
    }
    //finished thread initialization

    int taskrowinterval = this->height/this->m_thread_num;
    this->taskarray = new Task*[m_thread_num];
    for (int i = 0; i < m_thread_num-1; ++i) {
        taskarray[i] = new Task;
        taskarray[i]->start_row = i*taskrowinterval;
        taskarray[i]->end_row = (i+1)*taskrowinterval-1;
        taskarray[i]->field = this->field;
        taskarray[i]->tempfield = this->tempfield;
        taskarray[i]->field_width = this->width;
        taskarray[i]->field_height = this->height;
        taskarray[i]->phase = 1;
    }
    taskarray[m_thread_num-1] = new Task;
    taskarray[m_thread_num-1]->start_row = (m_thread_num-1)*taskrowinterval;
    taskarray[m_thread_num-1]->end_row = this->height-1;
    taskarray[m_thread_num-1]->field = this->field;
    taskarray[m_thread_num-1]->tempfield = this->tempfield;
    taskarray[m_thread_num-1]->field_width = this->width;
    taskarray[m_thread_num-1]->field_height = this->height;
    taskarray[m_thread_num-1]->phase = 1;
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field 
	// NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade

    //finished making tasks
    for (int i = 0; i < m_thread_num; ++i) {
        task_queue->push(this->taskarray[i]);
        this->barrier->increase();
    }
    barrier->wait();
    for (int i = 0; i < m_thread_num; ++i) {
        taskarray[i]->phase=2;
    }
    for (int i = 0; i < m_thread_num; ++i) {
        task_queue->push(this->taskarray[i]);
        this->barrier->increase();
    }
    barrier->wait();
    /*for (int i = 0; i < m_thread_num; ++i) {
        taskarray[i]->phase=1;
    }*/
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// All threads must be joined here
    Task* taskarray[m_thread_num];
    for (int i = 0; i < m_thread_num; ++i) {
        taskarray[i] = new Task;
        taskarray[i]->start_row=-1;
        task_queue->push(taskarray[i]);
    }
	for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }
    for (uint i = 0; i < m_thread_num; ++i) {
        delete taskarray[i];
    }
    for (int i = 0; i < height; ++i)
        delete[] field[i];
    for (int i = 0; i < height; ++i)
        delete[] tempfield[i];
    delete[] field;
    delete[] tempfield;

}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;
		
		// TODO: Print the board 
        cout << u8"╔" << string(u8"═") * width << u8"╗" << endl;
        for (uint i = 0; i < height; ++i) {
            cout <<u8"║";
            for (uint j = 0; j < width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
            }
            cout << u8"║" << endl;
        }
        cout << u8"╚" << string(u8"═") * width << u8"╝" << endl;
		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}
Game::Game(game_params params) {
    this->m_thread_num = params.n_thread;
    this->m_gen_num = params.n_gen;
    this->print_on = params.print_on;
    this->interactive_on = params.interactive_on;
    this->filename = params.filename;
    this->task_queue = new PCQueue<Task*>();
    // TODO continue working on constructor TODO
    pthread_mutex_init(&this->mutex,NULL);
    barrier = new Barrier;
}
const vector<double> Game::gen_hist() const {
    return this->m_gen_hist;
}
const vector<double> Game::tile_hist() const {
    return this->m_tile_hist;
}
uint Game::thread_num() const {
    return this->m_thread_num;
}


/* Function sketch to use for printing the board. You will need to decide its placement and how exactly 
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/ 



