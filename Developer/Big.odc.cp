/* parallel version of BUGS that does one MCMC sweep using BugsBig library	*/

#include "mpi.h"
#include "BugsBig.h"

main(int argc, char** argv){
	int			process;
	int			numPro;
	int			numSampler;
	int			i;
	int			j;
	int			type;
	int			label;
	int			accept;
	double			oldVal;
	double 			newVal;
	double			localLL;
	double 			oldLL;
	double			newLL;
	double			localP[2];
	double			p[2];
	int			labels[];
	double			values[];
	
	int	singleProcessor = 0;
	int	metropolis = 1;
	int	conjugateUV = 2;
	int root = 0;
	
	/* start MPI stuff	*/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_Comm_World, &numPro);
	MPI_Comm_rank(MPI_Comm_Worl, &process);
	
	numSamplers = BugsBig_NumUpdaters();
	for(i = 0; i < numSampler; i++){
		BugsBig_GetUpdater(); /*get updater from update list on each processor*/
		label = BugsBig_GetLabel();
		type = BugsBig_GetType();
		if(type == singleProcessor){ /* sampling different parameters on different processor*/
			BugsBig_Sample();
			newVal = BugsBig.GetValue();
			MPI_Allgather(label, 1, labels, 1, MPI_INT);
			MPI_Allgather(newVal, 1, values, MPI_DOUBLE);
			for(j = 0; j < numPro; j++){
				BugsBig_SetValue(labels[j], values[j])
			}
		}
		elsif(type == metropolis){	/* spread metropolis algorithm over all the processors*/
			oldVal = BugsBig_GetValue();
			locallLL = BugsBig_LogLikelihood();
			MPI_Reduce(localLL, oldLL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if(process == 0){
				BugsBig_Sample(); /* generate candidate point*/ /* calculate old log likelihood*/
				newVal := BugsBig_GetValue();
			}
			MPI_Bcast(newValue, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			BugsBig_SetValue(label, newVal);
			localLL = BugsBig_LogLikelihood(); /*calculate new log likelihood*/
			MPI_Reduce(localLL, newLL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if(process == 0){
				accept = BugsBig_Accept(newLL - oldLL) /* metropolis test*/
			}
			MPI_Bcast(accept, 1, MPI_INT, 0, MPI_COMM_WORLD);
			if(accept == 0){
				BugsBig_SetValue(label, oldVal) /*reject candidate point*/
			}
		}
		elsif(type == conjugateUV){
			BugsBig_LikelihoodForm(localP[0], localP[1]); /*calculate conjugate parameters*/
			MPI_Reduce(localP, p, 2, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if(process == 0){
				BugsBig_SetLikelihoodParams(p[0], p[1])
				BugsBig_Sample(); /*sample from conjugate distribution*/
				newVal := BugsBig.GetValue();
			}
			MPI_Bcast(newVal, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			BugsBig_SetValue(label, newVal)
		}
		MPI_Barrier(MPI_COMM_WORLD); /* make sure all process have finished this sampler before
		getting the next sampler*/
		
	}
	
	MPI_Finalize()
	
}
	
	
