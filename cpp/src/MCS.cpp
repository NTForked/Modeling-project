#include "MCS.h"


//Constructor
MCS::MCS(const int n_rows, const int n_cols, const int n_stacks):   
    N_ROWS(n_rows),N_COLS(n_cols),N_STACKS(n_stacks),
    N_TYPE1((N_ROWS-1)*N_COLS),
    N_TYPE2((N_ROWS-1)*(N_COLS-1)),
    N_TYPE3(N_ROWS*(N_COLS-1)),
    N_TYPE4(N_TYPE2),
    N_PARTICLES(N_ROWS*N_COLS*N_STACKS),
    N_CONNECTIONS(N_TYPE1+N_TYPE2+N_TYPE3+N_TYPE4),
    particles(n_rows*n_cols*n_stacks)
{
	initParticles();
    initConnections();
}


void MCS::initParticles(){
// Set starting values to masses, positions velocities and forces
    for (int i = 0; i < N_PARTICLES; ++i)
    {
        int row = i%N_COLS;                         //x
        int col = (i/N_COLS)%N_ROWS;              //y
        int stack = i/(N_COLS*N_ROWS);              //z

        // starting position from 3D index, vel. = 0
        particles[i] = Particle(1.0f, glm::vec3(row,col,stack)); 

    }
    particles[0].storeForce(glm::vec3(0,0,1000));
}

void MCS::initConnections(){
    std::vector<Connection> c_tmp(N_CONNECTIONS);
    connections = c_tmp;

    // Calculate connections
    int p_index1;
    int p_index2;
    for (int i = 0; i < N_CONNECTIONS; ++i){
        connection2massIndices3D(i, p_index1, p_index2, N_ROWS, N_COLS, N_STACKS);

        connections[i] = Connection(&particles[p_index1], &particles[p_index2]);
        connections[i].setSpringConstant(2000.f);
        connections[i].setDamperConstant(5.f);

    }
}

void MCS::update(float dt){
    for (int i = 0; i < N_CONNECTIONS; ++i)
    {
        connections[i].applyForcesToConnectedParticles(dt);
    }

    for (int i = 0; i < N_PARTICLES; ++i)
    {
        particles[i].applyForce(dt);
    }
}

//Get-functions
const Particle& MCS::getParticle(int index) const{
    return particles[index];
}

const Connection& MCS::getConnection(int index) const{
    return connections[index];
}

int MCS::numberOfParticles(){
    return N_PARTICLES;
}

int MCS::numberOfConnections(){
    return N_CONNECTIONS;
}
