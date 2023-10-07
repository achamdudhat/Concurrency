#include "channel.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */
    channel_t* channel=(channel_t*)malloc(sizeof(channel_t));
    pthread_mutex_init(&channel->mutex,NULL);//lock
    sem_init(&channel->semaphore_msg_buffer,0,0);//track wait list for recieving msg 
    sem_init(&channel->semaphore_total_cap,0,(unsigned int)size); //track capacity for sending msg 
    
    
    sem_init(&channel->select_data,0,1); 
    
    
    
    channel->closed_channel=false; 
    
    
    if(size<0){ 
        channel->buffered_channel=false;
        printf("Invalid size for creating channel");
        return NULL;
    }
    
    if(size==0){
        channel->buffered_channel=false;
        printf("Size is 0 and not valid");
        return NULL;
    }
    
    if(size>0){
        channel->buffer=buffer_create(size);
        channel->buffered_channel=true;
    }
   
    return channel;
}

// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_send(channel_t *channel, void* data)
{  
    /* IMPLEMENT THIS */
    
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);


    sem_wait(&channel->semaphore_total_cap);//-1



    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
    pthread_mutex_unlock(&channel->mutex);
    sem_post(&channel->semaphore_total_cap);
    return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);


    pthread_mutex_lock(&channel->mutex);
    int buff_result=buffer_add(channel->buffer,data);
    sem_post(&channel->semaphore_msg_buffer);
    pthread_mutex_unlock(&channel->mutex);
    

    pthread_mutex_lock(&channel->mutex);
    if(buff_result==BUFFER_ERROR){
        return GEN_ERROR;
    }    
    pthread_mutex_unlock(&channel->mutex);
    

//100  0
//99   1
   // printf("Value: %d",&(int*)channel->temp_semdata);
    //sem_post((sem_t*)channel->temp_semdata);
    return SUCCESS;

}

// Reads data from the given channel and stores it in the function's input parameter, data (Note that it is a double pointer)
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_receive(channel_t* channel, void** data)
{
   // void temp=*data;
    /* IMPLEMENT THIS */
    
    void *temp=*(&data);
    
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
    pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }    
    pthread_mutex_unlock(&channel->mutex);
    
    sem_wait(&channel->semaphore_msg_buffer);//-1

    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
    pthread_mutex_unlock(&channel->mutex);
        sem_post(&channel->semaphore_msg_buffer);
        return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);




    pthread_mutex_lock(&channel->mutex);
    int buff_result=buffer_remove(channel->buffer,temp);
    pthread_mutex_unlock(&channel->mutex);
    
    sem_post(&channel->semaphore_total_cap);
    
    
    
    
    pthread_mutex_lock(&channel->mutex);
    if(buff_result==BUFFER_ERROR){
        return GEN_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);
   
   // sem_post((sem_t*)channel->temp_semdata);
    


    return SUCCESS;
}

// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
            pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);

    if(sem_trywait(&channel->semaphore_total_cap)==-1){
            return CHANNEL_FULL;
    }//-1

    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
            pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);


    pthread_mutex_lock(&channel->mutex);
    int buff_result=buffer_add(channel->buffer,data);
    sem_post(&channel->semaphore_msg_buffer);
    pthread_mutex_unlock(&channel->mutex);
    

    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
            pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);
   
    
    
    
    pthread_mutex_lock(&channel->mutex);
    if(buff_result==BUFFER_ERROR){
        return GEN_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);
    
    //sem_post((sem_t*)channel->temp_semdata);

return SUCCESS;
   

}

// Reads data from the given channel and stores it in the function's input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */

    void *temp=*(&data);
    
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }   
    pthread_mutex_unlock(&channel->mutex);
    
 //   sem_trywait(&channel->semaphore_msg_buffer);//-1

 if(sem_trywait(&channel->semaphore_msg_buffer)==-1){
        return CHANNEL_EMPTY;
    }//-1

    pthread_mutex_lock(&channel->mutex); 
    if(channel->closed_channel==true){
            pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);


    pthread_mutex_lock(&channel->mutex);    
    int buff_result=buffer_remove(channel->buffer,temp);
    pthread_mutex_unlock(&channel->mutex);

    sem_post(&channel->semaphore_total_cap);
    

    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
            pthread_mutex_unlock(&channel->mutex);
           return CLOSED_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);

    
    pthread_mutex_lock(&channel->mutex);
    if(buff_result==BUFFER_ERROR){
        return GEN_ERROR;
    }
    pthread_mutex_unlock(&channel->mutex);
    //sem_post((sem_t*)channel->temp_semdata);

    return SUCCESS;
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GEN_ERROR in any other error case
enum channel_status channel_close(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed_channel==true){
        pthread_mutex_unlock(&channel->mutex);    
        return CLOSED_ERROR;
    }
    channel->closed_channel=true;

    pthread_mutex_unlock(&channel->mutex);    
    sem_post(&channel->semaphore_msg_buffer);
    sem_post(&channel->semaphore_total_cap);

    sem_close(&channel->select_data);
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
    /* IMPLEMENT THIS */
    if(channel->closed_channel==false){
        return DESTROY_ERROR;
    }
    pthread_mutex_destroy(&channel->mutex);
    sem_destroy(&channel->select_data);
    
    buffer_free(channel->buffer);
    //channel_close(channel);
    free(channel);
    return SUCCESS;
}

// Takes an array of channels (channel_list) of type select_t and the array length (channel_count) as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */

while(true){
    for(size_t i=0;i<channel_count;i++){
        
        //channel_list[i].channel->temp_semdata=&operations_done;

        //pthread_mutex_lock(&channel_list[i].channel->mutex);
        if(channel_list[i].dir==SEND){

            int send_result=channel_non_blocking_send(channel_list[i].channel,channel_list[i].data);


            if(send_result==1){
                *selected_index=i;
                //pthread_mutex_unlock(&channel_list[i].channel->mutex);
                sem_post(&channel_list[i].channel->select_data);
                return SUCCESS;
            }
            else if((send_result==0)){
            
             //   pthread_mutex_unlock(&channel_list[i].channel->mutex);
               // sem_wait(&operations_done);
                continue;
            }
            else if(send_result==-2){
                *selected_index=i;
                //   pthread_mutex_unlock(&channel_list[i].channel->mutex);
                sem_post(&channel_list[i].channel->select_data);
                return CLOSED_ERROR;
            }
            else {
                //*selected_index=i;
                //pthread_mutex_unlock(&channel_list[i].channel->mutex);
                sem_post(&channel_list[i].channel->select_data);
                return GEN_ERROR;
            }

        }



        if(channel_list[i].dir==RECV){

            int recv_result=channel_non_blocking_receive(channel_list[i].channel,&channel_list[i].data);
        
            if(recv_result==1){            
                 *selected_index=i;
                  //pthread_mutex_unlock(&channel_list[i].channel->mutex);
                  sem_post(&channel_list[i].channel->select_data);         
                  return SUCCESS;
            }
            else if((recv_result==0)){
                //pthread_mutex_unlock(&channel_list[i].channel->mutex);  
                //sem_wait(&operations_done);               
                continue;
            }
            else if(recv_result==-2){
                *selected_index=i;
                sem_post(&channel_list[i].channel->select_data);
                return CLOSED_ERROR;

            }
            else {   
                    sem_post(&channel_list[i].channel->select_data);
                    return GEN_ERROR;
            }
        }
    
        //pthread_mutex_unlock(&channel_list[i].channel->mutex);
        sem_wait(&channel_list[i].channel->select_data);
        //sem_destroy(&channel_list[i].channel->select_data);
   
    }
    }
    //sem_wait(&operations_done);

    //sem_destroy(&operations_done);
   return SUCCESS;
}