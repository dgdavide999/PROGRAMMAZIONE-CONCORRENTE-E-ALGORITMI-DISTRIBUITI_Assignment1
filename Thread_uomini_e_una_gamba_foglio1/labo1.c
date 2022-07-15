#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

double **A,**B,**C;
double **AB, **CAB;
int M,N,P,block_size;

pthread_barrier_t barrier;
pthread_mutex_t* mutex;


void fail(const char *const msg){
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
void fail_errno(const char *const msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

void empty_matrix_initializer(){
    if((A=(double **)malloc(M*sizeof(double*)))==NULL){
        fail("A allocation failed\n");
    }
    for(int i = 0;i<M;i++)
    	if((A[i]= malloc(N*sizeof(double)))==NULL){
          fail("A allocation failed\n");
    }

    if((B =(double **)malloc(N*sizeof(double*)))==NULL){
        fail("B allocation failed\n");
    }
    for(int i = 0;i<N;i++)
    	if((B[i]= malloc(P*sizeof(double)))==NULL){
            fail("B allocation failed \n");
    }

    if((C=(double **)malloc(P*sizeof(double*)))==NULL){
        fail("C allocation failed\n");
    }
    for(int i = 0;i<P;i++)
    	if((C[i]= malloc(M*sizeof(double)))==NULL){
            fail("C allocation failed\n");
    }

    if((AB=(double **)malloc(M*sizeof(double*)))==NULL){
        fail("AB allocation failed\n");
    }
    for(int i = 0;i<M;i++)
    	if((AB[i]= malloc(P*sizeof(double)))==NULL){
            fail("AB allocation failed\n");
    }

    if((CAB=(double **)malloc(P*sizeof(double*)))==NULL){
        fail("CAB allocation failed\n");
    }
    for(int i = 0;i<P;i++)
    	if((CAB[i]= malloc(P*sizeof(double)))==NULL){
            fail("CAB allocation failed\n");
    }
}
void matrix_filler(){

    for(int i = 0;i<M;i++){
        for (int j = 0;j < N; j++)
        {
           A[i][j]=1.5;
        }
    }
    for(int i = 0;i<N;i++){
        for (int j = 0; j < P; j++)
        {
           B[i][j]=2.5;
        }
    }

    for(int i = 0;i<P;i++){
        for (int j = 0; j < M; j++)
        {
           C[i][j] = 3.5;
        }
    }    
}
int initialize_data(){
    printf("A è una matrice MxN,\nB è una matrice NxP,\nC è una matrice PxM\ninserisci M,N,P\n");
    scanf("%d", &M);
    scanf("%d", &N);
    scanf("%d", &P);

    empty_matrix_initializer();
    matrix_filler();

    if((mutex = (pthread_mutex_t*)malloc(P*sizeof(pthread_mutex_t)))== NULL){
        fail("mutex allocation failed\n");
        return -1;
    }    
    for (int i = 0; i < P; i++)
    {
       if(pthread_mutex_init(mutex+i, NULL)!=0){
           fail_errno("mutex initialize failed\n");
       }
    }

    printf("numero di righe per blocco? ");
    scanf("%d", &block_size);
    if(block_size<=0 || M%block_size != 0){
        printf("il numero di righe scelto non è un divisore di M o è minore o uguale a 0\n");
        return(-1);
    }
    return 0;
}


void AxB(int n_block){
    for(int m1_line = n_block; m1_line < n_block+block_size; m1_line++)
    {
        for (int m2_c = 0; m2_c < P; m2_c++)
        {
            double val = 0;
            for (int i = 0; i < N; i++)
            {
                val+= A[m1_line][i] * B[i][m2_c];
            }
            AB[m1_line][m2_c]= val;
        }
        
    }
}
void CxR(int n_block){
    for (int m1_l = 0; m1_l < P; m1_l++)
    {
        for (int m2_c = 0; m2_c < P; m2_c++)
        {
            for (int i = n_block; i < n_block+block_size; i++)
            {
                if(pthread_mutex_lock(mutex+m2_c)!=0)
                    fail_errno("lock failed\n");

                CAB[m1_l][m2_c] += C[m1_l][i] * AB[i][m2_c];
                if(pthread_mutex_unlock(mutex+m2_c)!=0)
                    fail_errno("unlock failed\n");
            }
        }
        
    }
}
void *blocco(void* n){
    int n_block = *((int *)n);
    AxB(n_block);
    pthread_barrier_wait(&barrier);
    CxR(n_block);
    return 0;
}

void print_result(){
    printf("\n\n");
    for(int i = 0;i<P;i++){
        for (int j = 0; j < P; j++)
        {
           printf("%f ",CAB[i][j]);
        }
        printf("\n");
    }
}
void free_all(void* p1,void* p2){
    for(int i=0; i<M; i++){
        free(A[i]);
    }
    free(A);
    for(int i=0; i<N; i++){
        free(B[i]);
    }
    free(B);
    for(int i=0; i<P; i++){
        free(C[i]);
    }
    free(C);
    for(int i=0; i<M; i++){
        free(AB[i]);
    }
    free(AB);
    for(int i=0; i<P; i++){
        free(CAB[i]);
    }
    free(CAB);
    free(mutex);
    free(p1);
    free(p2);
}

int main() {
    if(initialize_data()<0)return -1;
    int n_thread = M/block_size;
    printf("thread usati: %d\n", n_thread);
    pthread_t tid[n_thread];
    void* ret;
    int *n_block;
    if((n_block = malloc(n_thread*sizeof(int)))==NULL){
            fail("malloc failed\n");
        }
    if(pthread_barrier_init(&barrier, NULL, n_thread+1)!=0){
        fail_errno("barrier initialization failed\n");
    }
    
    struct timespec ts, te;
    timespec_get(&ts, TIME_UTC);

    int j=0;
    for(int i=0; i<M; i+=block_size){ 
        n_block[j] = i;
        if(pthread_create(tid+j++,NULL,blocco,n_block+j)!=0)
            fail_errno("thread creation failed\n");
    }   
    
    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);

    for(int i=0; i<n_thread;i++)
        if(pthread_join(tid[i],&ret)!=0)
            fail_errno("join failed for thread\n");
    
    timespec_get(&te, TIME_UTC);
    if(te.tv_sec - ts.tv_sec < 0.000001)
        printf("Time taken is %ld nanoseconds\n",te.tv_nsec - ts.tv_nsec);
    else
        printf("Time taken is %ld seconds\n",(te.tv_sec - ts.tv_sec));
    free_all(n_block,ret);
}