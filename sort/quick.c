/* quicksort */
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

#define N 10

void showElapsed(int id, char *m);
void showVector(int *v, int n, int id);
int * merge(int *v1, int n1, int *v2, int n2);
void swap(int *v, int i, int j);
void qsort2(int *v, int left, int right);

double startTime, stopTime;

void showElapsed(int id, char *m)
{
	printf("%d: %s %f secs\n",id,m,(clock()-startTime)/CLOCKS_PER_SEC);
}

void showVector(int *v, int n, int id)
{
	int i;
	printf("%d: ",id);
	for(i=0;i<n;i++)
		printf("%d ",v[i]);
	putchar('\n');
}

int * merge(int *v1, int n1, int *v2, int n2)
{
	int i,j,k;
	int * result;

	result = (int *)malloc((n1+n2)*sizeof(int));

	i=0; j=0; k=0;
	while(i<n1 && j<n2)
		if(v1[i]<v2[j])
		{
			result[k] = v1[i];
			i++; k++;
		}
		else
		{
			result[k] = v2[j];
			j++; k++;
		}
	if(i==n1)
		while(j<n2)
		{
			result[k] = v2[j];
			j++; k++;
		}
	else
		while(i<n1)
		{
			result[k] = v1[i];
			i++; k++;
		}
	return result;
}

void swap(int *v, int i, int j)
{
	int t;
	t = v[i];
	v[i] = v[j];
	v[j] = t;
}

void qsort2(int *v, int left, int right)
{
	int i,last;
	if(left>=right)
		return;
	swap(v,left,(left+right)/2);
	last = left;
	for(i=left+1;i<=right;i++)
		if(v[i]<v[left])
			swap(v,++last,i);
	swap(v,left,last);
	qsort2(v,left,last-1);
	qsort2(v,last+1,right);
}

void my_bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator) {
  int world_rank;
  MPI_Comm_rank(communicator, &world_rank);
  int world_size;
  MPI_Comm_size(communicator, &world_size);

  if (world_rank == root) {
    // Se processo raiz, enviar o dado para todos
    int i;
    for (i = 0; i < world_size; i++) {
      if (i != world_rank) {
        MPI_Send(data, count, datatype, i, 0, communicator);
      }
    }
  } else {
    // Se processo receptor, receber o dado da raiz
    MPI_Recv(data, count, datatype, root, 0, communicator,
             MPI_STATUS_IGNORE);
  }
}


int main(int argc, char **argv)
{
	int * data;
	int * chunk;
	int * other;
	int m,n=N;
	int id,p;
	int s;
	int i;
	int step;
	MPI_Status status;


	startTime = clock();

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&p);

	showElapsed(id,"MPI setup complete");

	if(id==0)
	{
		int r;
		srandom(clock());
		
		s = n/p;
		r = n%p;
		data = (int *)malloc((n+s-r)*sizeof(int));
		for(i=0;i<n;i++)
			data[i] = random();
		if(r!=0)
		{
			for(i=n;i<n+s-r;i++)
				data[i]=0;
			s=s+1;
		}
		showElapsed(id,"generated the random numbers");


	
		
		my_bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);
		chunk = (int *)malloc(s*sizeof(int));

//		MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
		for(i=1;i<s;i++)
		MPI_Send(data,s,MPI_INT,i,0,MPI_COMM_WORLD);
		


		showElapsed(id,"scattered data");

		qsort2(chunk,0,s-1);

		showElapsed(id,"sorted");

	
	}
else {	

		my_bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);

		chunk = (int *)malloc(s*sizeof(int));


		MPI_Recv(chunk,s,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);

		//MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
		


		showElapsed(id,"got data");

		qsort2(chunk,0,s-1);

		showElapsed(id,"sorted");


}
			
	step = 1;
	while(step<p)
	{
		if(id%(2*step)==0)
		{
			if(id+step<p)
			{	
				MPI_Recv(&m,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
				other = (int *)malloc(m*sizeof(int));
				MPI_Recv(other,m,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
				showElapsed(id,"got merge data");
				chunk = merge(chunk,s,other,m);
				showElapsed(id,"merged data");
				s = s+m;
			} 
		}
		else
		{
			int near = id-step;
			MPI_Send(&s,1,MPI_INT,near,0,MPI_COMM_WORLD);
			MPI_Send(chunk,s,MPI_INT,near,0,MPI_COMM_WORLD);
			showElapsed(id,"sent merge data");
			break;
		}
		step = step*2;
	}
	if(id==0)
	{
		FILE * fout;

		stopTime = clock();
		printf("%d; %d processors; %f secs\n", s,p,(stopTime-startTime)/CLOCKS_PER_SEC);

		showElapsed(id,"opening out file");
		fout = fopen("result","w");
		for(i=0;i<s;i++)
			fprintf(fout,"%d\n",chunk[i]);
		fclose(fout);
		showElapsed(id,"closed out file");
	}
	MPI_Finalize();
}
