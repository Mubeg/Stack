#include "Stack.h"

int main(){

	#ifdef TESTING
	if(!test_stack()){
		fprintf(stderr, "Stack not passed test (tested from %s line %d in %s)\n", __LOCATION__);
	}
	#endif

	//Stack_t stack_for_testing = {};
	//Stack_t * stack_for_testing = &stack; 
	
	/*if(!STACK_INIT(&stack_for_testing)){
		fprintf(stderr, "stack_init() return false\n");
		return 1;
	}

	if(!stack_push(&stack_for_testing, 10)){
		fprintf(stderr, "stack_push() return false\n");
		return 1;
	}*/

	//int error = 0;
	/*Elem_t temp = stack_pop(&stack_for_testing, &error);
	if(error){
		stack_dump(&stack_for_testing, __LOCATION__, false);
		return error;	
	}

	if(!stack_deinit(&stack_for_testing)){
		fprintf(stderr, "stack_deinit() return false\n");
		return 1;	
	}*/

	return 0;

}

bool test_stack(void){

	Torture_t torture = {};

	STACK_INIT(&torture.stk);

	Buff_elem_t * buff = nullptr;
	int buff_size = 0;
	buff = read_file_to_created_buff("torture.txt", &buff_size);

	if(buff == nullptr){
		fprintf(stderr, "Not able to create buff in %s line %d in %s\n", __LOCATION__);
		return false;
	}

	int commands_size = 0;
	str_ptr commands = make_text_must_free(buff, buff_size, &commands_size);

	if(commands == nullptr){
		fprintf(stderr, "Not able to create commands in %s line %d in %s\n", __LOCATION__);
		return false;
	}

	commands_size = 1;
	for(int i = 0; i < commands_size; i++){

		int test_id = 0;
		bool passed = true;	

		test_id = get_test(commands[i]);
		switch (test_id) {
			case 0:
				break;
			case 1:
				torture.first_kill[3] = '\0';
				passed = check_stack(&torture.stk, __LOCATION__);
				break;
			case 2:
				for(int i = 0; i < 2*STACK_INIT_SIZE + 1; i++) stack_push(&torture.stk, 10);
				passed = check_stack(&torture.stk, __LOCATION__);
				break;
			default:
				fprintf(stderr, "Unexpected return of get_test in %s line %d in %s\n", __LOCATION__);
				break;
		}

		if(!passed){
			fprintf(stderr, "Test #%d not passed. %s line %d in %s\n", test_id, __LOCATION__);
			return false;
		}
	}

	stack_deinit(&torture.stk);
	free(buff);
	free(commands);

	return true;
}

int get_test(str command_line){

	return 2;
}

Buff_elem_t *read_file_to_created_buff(const char file_name[], int *buff_size){

	assert(file_name[0] != '\0');
	assert(file_name);

	FILE *file = fopen(file_name, "r");
	if(!file){
		fprintf(stderr, "Not able to open %s in %s line %d in %s\n", file_name, __LOCATION__);
		return nullptr;
	}
	
	const int file_length = get_file_length(file, DEFAULT);
	if(file_length < 0){
		fprintf(stderr, "get_file_length() return incorrect\n");
		return nullptr;	
	}

	*buff_size = file_length + 1;
	Buff_elem_t *buff = (Buff_elem_t *) calloc(file_length + 1, sizeof(buff[0]));

	int num_read = read(fileno(file), buff, file_length*sizeof(Buff_elem_t)); // ? < 4k
	if(num_read == -1 || num_read != file_length){
		return nullptr;
	}

	fclose(file);
	return buff;
}

int get_file_length(FILE *file, const int mode){

	assert(file != nullptr);

	int ans = 0;

	if(fseek(file, 0, SEEK_END)){
		fprintf(stderr, "fseek in %s SEEK_END return not zero\n", __PRETTY_FUNCTION__);
	}
	if((ans = ftell(file)) == -1L){
		fprintf(stderr, "ftell in %s return -1L\n", __PRETTY_FUNCTION__);
	}
	if(fseek(file, 0, SEEK_SET)){
		fprintf(stderr, "fseek in %s SEEK_SET return not zero\n", __PRETTY_FUNCTION__);
	}
	
	return ans;
}

str_ptr make_text_must_free(Buff_elem_t buff[], const int buff_size, int * const text_size){

	assert(buff      != nullptr);
	assert(text_size != nullptr);
	assert(buff_size >= 0);

	int num_lines = 0;

	num_lines = change_ch1_to_ch2_and_count_in_ch_buff(Long_string('\n'), Long_string('\0'), buff, buff_size);

	str_ptr text = nullptr;
	text = (str_ptr) calloc(num_lines + 1, sizeof(str)); // text[num_lines].size = -1
	if(!text){
		fprintf(stderr, "Mem calloc for text in %s problem", __PRETTY_FUNCTION__);
	}

	*text_size = fill_string_split_by_separator_and_skip_non_alpha_lines_from_ch_buff(text, Long_string('\0'), buff, buff_size);

	text[*text_size].size = -1;
	
	return text;
}

int change_ch1_to_ch2_and_count_in_ch_buff(const Buff_elem_t char1, const Buff_elem_t char2, Buff_elem_t buff[], const int buff_size){

	assert(buff      != nullptr);
	assert(buff_size >= 0      );

	int num_lines = 0;

	for(Buff_elem_t * ptr = (Buff_elem_t *) wmemchr(buff, char1, buff_size);
	    ptr < buff + buff_size;
	    ptr = (Buff_elem_t *) wmemchr(ptr + 1, char1, buff + buff_size - 1 - ptr)){

		if(!ptr)
			break;
	
		num_lines++;
		*ptr = char2;
	}
	
	return num_lines;
}

int fill_string_split_by_separator_and_skip_non_alpha_lines_from_ch_buff(str_ptr text, const Buff_elem_t separator, Buff_elem_t buff[], const int buff_size){
	
	assert(text      != nullptr);
	assert(buff      != nullptr);
	assert(buff_size >= 0      );

	bool found = false;
	bool has_alpha = false;
	int text_ptr = 0;
	int last_i = 0;

	for(int i = 0; i < buff_size; i+= sizeof(buff[0])){

		if(found){
			if(has_alpha){
				text[text_ptr].str = buff + last_i;
				text[text_ptr].size = i - last_i;
				text_ptr++;
				has_alpha = false;
			}
			last_i = i;
			found = false;
		}
		if(buff[i] == separator){
			found = true;
		}
		if(iswalpha(buff[i])){
			has_alpha = true;
		}
		
	}
	
	return text_ptr - 1;
}

int fill_string_split_by_separator_from_ch_buff(str_ptr text, const Buff_elem_t separator, Buff_elem_t * buff, const int buff_size){
	
	assert(text      != nullptr);
	assert(buff      != nullptr);
	assert(buff_size >= 0      );

	int text_ptr = 0;
	text[0].str = buff;
	Buff_elem_t * last_ptr = buff;

	for(Buff_elem_t * ptr = (Buff_elem_t *) wmemchr(buff, separator, buff_size);
	    ptr < buff + buff_size;
	    ptr = (Buff_elem_t *) wmemchr(last_ptr, separator, buff + buff_size - 1 - ptr)){
		
		if(ptr == nullptr)
			break;

		text_ptr++;
		text[text_ptr - 1].str = last_ptr;
		text[text_ptr - 1].size = ptr - last_ptr + 1;
		last_ptr = ptr + 1;
	}

	return text_ptr - 1;
}

bool stack_init(Stack_t *stk, const char name[], const size_t init_stack_size /*= STACK_INIT_SIZE*/){

	assert(check_nullptr((void *) stk));
	if(init_stack_size < 0){
		return false;	
	}

	stk->canary1 = CANARY_VALUE;
	
	stk->maxsize = init_stack_size;

	stk->buff = (char *) calloc(stk->maxsize*sizeof(Elem_t) + 2*sizeof(Elem_t), sizeof(char));
	if(stk->buff == nullptr){
		return false;
	}
	
	set_dynamic_canaries_and_data(stk);
	fill_data_with_poison(stk->data, stk->maxsize);
	stk->false_poison = 0;

	stk->size = 0;

	stk->name = name[0] == '&' ? name + 1: name;

	stk->canary2 = CANARY_VALUE;

	stk->hash = find_sum_hash(stk, sizeof(Stack_t));

	assert(check_stack(stk, __LOCATION__));
 
	return true;
}

bool stack_push(Stack_t *stk, Elem_t elem){
	
	assert(check_stack(stk, __LOCATION__));
	
	if(!stack_resize_if_needed(stk, /*is_increasing = true*/ true)){
		assert(check_stack(stk, __LOCATION__));
		return false;	
	}

	if(elem == POISON){
		stk->false_poison++;
	}

	stk->data[stk->size++] = elem;	
	stk->hash = find_sum_hash(stk, sizeof(Stack_t));
	
	assert(check_stack(stk, __LOCATION__));

	return true;
}

Elem_t stack_pop(Stack_t *stk, int *err/* = nullptr(*/){

	assert(check_stack(stk, __LOCATION__));

	if(!stack_resize_if_needed(stk, /*is_increasing = false*/ false)){
		assert(check_stack(stk, __LOCATION__));
		return false;	
	}
	
	if(stk->size == 0){
		if(err != nullptr){
			*err = 1;
		}
		assert(check_stack(stk, __LOCATION__));
		return POISON;	
	}


	Elem_t temp = stk->data[--stk->size];
	if(temp == POISON){
		stk->false_poison--;
	}
	stk->data[stk->size] = POISON;
	stk->hash = find_sum_hash(stk, sizeof(Stack_t));

	assert(check_stack(stk, __LOCATION__));

	return temp;
}

bool stack_deinit(Stack_t *stk){

	assert(check_stack(stk, __LOCATION__));
	
	zero_data(stk->buff, 0, stk->maxsize*sizeof(Elem_t) + 2*sizeof(Canary_t));
	free(stk->buff);

	return true;
}

bool stack_resize_if_needed(Stack_t *stk, bool is_increasing/* = false*/){

	assert(check_stack(stk, __LOCATION__));
	
	if(is_increasing && stk->size == stk->maxsize){

		void * temp = recalloc_safe(stk->buff, stk->maxsize*sizeof(Elem_t) + sizeof(Canary_t), 
				       stk->maxsize*2*sizeof(Elem_t) + sizeof(Canary_t)*2, sizeof(char));
		if(temp == nullptr){
			return false;		
		}

		stk->buff = (char *)temp;
		stk->maxsize *= 2;
		set_dynamic_canaries_and_data(stk);
		fill_data_with_poison(stk->data + stk->size, stk->maxsize - stk->size);

		stk->hash = find_sum_hash(stk, sizeof(Stack_t));

	}
	if(!is_increasing && stk->size < stk->maxsize/4){

		void * temp = recalloc_safe(stk->buff, stk->maxsize*sizeof(Elem_t) + sizeof(Canary_t), 
				       stk->maxsize*sizeof(Elem_t)/2 + sizeof(Canary_t)*2, sizeof(char));
		if(temp == nullptr){
			fprintf(stderr, "realloc problem while making smaller ????\n");
			return false;
		}
		if(temp != stk->buff){
			fprintf(stderr, "WOW, realloc tryies to move data!\n!\n!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
		
		stk->buff = (char *)temp;
		stk->maxsize /= 2;
		set_dynamic_canaries_and_data(stk);

		stk->hash = find_sum_hash(stk, sizeof(Stack_t));
	}
	

	assert(check_stack(stk, __LOCATION__));

	return true;
}

void set_dynamic_canaries_and_data(Stack_t * stk){

	assert(stk != nullptr);

	stk->canary_dynamic1 = (Canary_t *) stk->buff;
	stk->canary_dynamic1[0] = CANARY_VALUE;

	stk->data = (Elem_t *)(stk->buff + sizeof(Canary_t));

	stk->canary_dynamic2 = (Canary_t *)(stk->buff + sizeof(Canary_t) + stk->maxsize*sizeof(Elem_t));
	stk->canary_dynamic2[0] = CANARY_VALUE;

}

bool fill_data_with_poison(Elem_t * const data, const size_t size){
	
	assert(data != nullptr);
	if(size < 0) return false;

	for(unsigned int i = 0; i < STACK_INIT_SIZE; i++){// !!!!!ERRROR MANUALLY CREATED i < size
		data[i] = POISON;
	}

	return true;

}

void * recalloc_safe(void * data, size_t prev_num, size_t num, size_t size){
	
	assert(data != nullptr);

	if(prev_num >= num){

		zero_data(data, num*size, prev_num*size);
		return realloc(data, num*size);
	}
	
	if(sizeof(char) != 1){
		assert('0xBEDA' != '0xBEDA');
	}
	
	char * new_data = (char *) calloc(num, size);
	if(new_data == nullptr){
		return nullptr;
	}

	for(int i = 0; i < prev_num*size; i++){
		new_data[i] = ((char *) data)[i];
	}

	zero_data(data, 0, prev_num*size);


	free(data);
	return (void *) new_data;
}

void zero_data(void * const data, const int start, const int end){

	assert(data != nullptr);
	assert(start < end);	

	for(int i = start; i < end; i++){
		((char *) data)[i] = '\0';
	}

}

bool check_stack(Stack_t *stk, const char called_from_file[], int line, const char func[], 
		 int cur_state_func/* = NO_CHECK_FUNCTION_FULLCHECK*/, bool print_all/* = false*/){

//Check for func checking correctly (from start to end)
	if(cur_state_func != NO_CHECK_FUNCTION_FULLCHECK){

		static int prev_state_func = END_FUNCTION;
		assert(prev_state_func != cur_state_func);
		prev_state_func = cur_state_func;

	}
//End check for func

	bool passed = true;

	if(!check_nullptr((void *) stk)){
		return false;
	}

// A Lot of checks below
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_canary(stk)                    ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_hash(stk)                      ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_size(stk)                      ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);


	passed = check_nullptr(stk->buff)              ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);
 
	passed = check_nullptr(stk->name)              ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(stk->canary_dynamic1)   ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(stk->canary_dynamic2)   ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(stk->data)              ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);


	passed = check_size_not_neg(stk->maxsize)      ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_dynamic_canaries_and_data(stk)  ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_size_not_neg(stk->false_poison) ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_number_of_poison(stk)           ? passed : false;
	if(DEBUG) printf("Passed: %d\n", passed);

/*	passed = check_canary(stk)? passed : false;
	passed = check_canary(stk)? passed : false;
	passed = check_canary(stk)? passed : false;
	passed = check_canary(stk)? passed : false;
	passed = check_canary(stk)? passed : false;
	passed = check_canary(stk)? passed : false;*/

// End of a Lot of checks
		
	if(!print_all || !passed){
		stack_dump(stk, called_from_file, line, func, passed);
	}

	return passed;
}


void stack_dump(Stack_t * const stk, const char called_from_file[], int line, const char func[], bool ok){

	assert(stk != nullptr);

	FILE *log_file = fopen(LOG_FILE, "a");
	if(log_file == nullptr){
		fprintf(stderr, "Not able to open %s in %s line %d in %s\n", LOG_FILE, __LOCATION__);
	}


	time_t cur_time = -1;
	time(&cur_time);

	fprintf(log_file, "\nTime and date: %s\n"
			  "Dumping stack from file %s, line %d, function %s [%s]\n"
			  "Stack [%p] %s {\n"
			  "\n\t\t%s canary1 = %X\n"
			  "\n\t\t%s hash    = %ld\n"
			  "\t\t%s size    = %d\n"
			  "\t\t%s maxsize = %d\n"
			  "\n\t\t%s buff = [%p]\n"
			  "\t\t%s canary_dynamic1 [%p] = %X\n"
			  "\t\t%s data = [%p]{\n", 
		ctime(&cur_time),
		called_from_file, line, func, ok ? "ok" : "ERROR!!!",
		stk, stk->name,
		typeid(stk->canary1).name(), stk->canary1,
		typeid(stk->hash).name(), stk->hash, 
		typeid(stk->size).name(), stk->size, 
		typeid(stk->maxsize).name(), stk->maxsize,
		typeid(stk->buff).name(), stk->buff, 
		typeid(stk->canary_dynamic1).name(), stk->canary_dynamic1, stk->canary_dynamic1[0],
		typeid(stk->data).name(), stk->data);
	fflush(log_file);	


	for(int i = 0; i < stk->maxsize; i++){
		fprintf(log_file, "\t\t\t %c data[%d] = %d %s\n", 
			i < stk->size ? '*' : ' ', i, stk->data[i], stk->data[i] == POISON ? "POISON?" : "");
	}
	fflush(log_file);
	
	fprintf(log_file, "\t\t}\n\t\t%s canary_dynamic2 [%p] = %X\n"
			  "\n\t\t%s canary2 = %X\n\n\t}\n\n", 
		typeid(stk->canary_dynamic2).name(), stk->canary_dynamic2, stk->canary_dynamic2[0], 
		typeid(stk->canary2).name(), stk->canary2);
	fflush(log_file);
	/*

	Time and date: ....
	Dumping stack from file main.cpp, line 123, function check() [ok] ([ERROR!!!])
	Stack [0x1124asda] stk1 {
	
		Canary_t canary1 = ...	

		hash_t hash    = ....
		size_t size    = ....
		size_t maxsize = ....

		char * buff = [0x231141]
		Canary_t * canary_dynamic1 [0x1425123] = ...
		Elem_t *data = [0x23123]{
			* data[0] = ..
			  data[1] = ...  POISON?
			....
		}
		Canary_t * canary_dynamic2 [0x2131331] = ... 

		Canary_t canary2 = ...
	
	}

	*/

	fclose(log_file);

}

bool check_number_of_poison(Stack_t * const stk){

	assert(stk != nullptr);

	size_t counter = 0;

	for(size_t i = 0; i < stk->maxsize; i++){
		counter += stk->data[i] == POISON ? 1 : 0;
	}
	
	return counter - stk->false_poison == stk->maxsize - stk->size;
}

bool check_nullptr(const void * const stk){
	
	if(stk == nullptr){
		return false;
	}
	
	return true;
}

bool check_canary(Stack_t * const stk){

	assert(stk != nullptr);
	return stk->canary1 == stk->canary2 && stk->canary2 == CANARY_VALUE;
}

bool check_size_not_neg(const size_t size){

	return size >= 0;
}

bool check_hash(Stack_t * const stk){

	assert(stk != nullptr);
	return is_equal_hash_t(find_sum_hash(stk, sizeof(Stack_t)), stk->hash);
}

bool check_dynamic_canaries_and_data(Stack_t * const stk){

	assert(stk != nullptr);
	assert(stk->canary_dynamic1 != stk->canary_dynamic2);
	assert(stk->canary_dynamic1 != nullptr);

	return !((char *)(stk->canary_dynamic2) - (char *)(stk->canary_dynamic1) != sizeof(Canary_t) + stk->maxsize*sizeof(Elem_t) 
		|| (char *)stk->data - (char *)stk->canary_dynamic1 != sizeof(Canary_t) 
		|| stk->canary_dynamic2[0] != stk->canary_dynamic1[0] 
		|| stk->canary_dynamic2[0] != CANARY_VALUE);

}

bool is_equal_hash_t(const hash_t left, const hash_t right){
	
	return left == right;
}

hash_t find_sum_hash(Stack_t * const stk, size_t size){

	assert(stk != nullptr);

	hash_t hash_sum = 0;
	
	hash_sum += hash(stk + sizeof(hash_t), size);
	hash_sum += hash(stk->buff, sizeof(stk->canary_dynamic1) + sizeof(stk->canary_dynamic2) + sizeof(stk->data[0])*stk->maxsize);
	hash_sum += hash(stk->name, strlen(stk->name));

	return hash_sum;

}

hash_t hash(const void * const stk, size_t size){

	assert(stk != nullptr);

	hash_t hash_sum = 0;

	for(size_t counter = 0; counter < size; counter++){
		hash_sum += (counter+1)*(*((char *) stk + counter));
	}

	return hash_sum;

}

bool check_size(Stack_t * const stk){
	
	assert(stk != nullptr);

	if(stk->size < 0 || stk-> size > stk->maxsize){
		return 0;
	}

	return true;
}















