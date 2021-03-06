#include "ElasticMaterials_lib/NumericalMethods.h"

/** Global functions  **/
glm::vec3 calculateEulerExplicit(){
	return glm::vec3(0,0,0);
}

glm::vec3 calculateRungeKutta(){
	return glm::vec3(0,0,0);
}


//EulerExplicit
void EulerExplicit::update(MCS& mcs, float dt){
	glm::vec3 new_velocity, new_position;
	glm::vec3 delta_v, delta_p;

	mcs.calcConnectionForcesOnParticles();
	for (int i = 0; i < mcs.getNumberOfParticles(); ++i){
        if (!mcs.lock_.particleLocked(i)){ // Detta är inte snyggt, numerical methods borde separeras mer från MCS. Gäller även kollisionen?
            mcs.calcAccelerationOfParticle(i);
            
            new_velocity = mcs.particles.velocities[i] + mcs.particles.accelerations[i] * dt;
            new_position = mcs.particles.positions[i] + new_velocity * dt;
            
            //Check collisions with collision planes
            mcs.checkCollisions(new_position, new_velocity);
            
            //Set new position and velocity
            mcs.particles.velocities[i] = new_velocity;
            mcs.particles.positions[i] = new_position;
        }
	}
}

//RungeKutta
RungeKutta::RungeKutta(std::vector<float> weights){
	setWeights(weights);
}

void RungeKutta::setWeights(std::vector<float> weights){
	w = weights;
	wSum = 0.0f;
    for (int i = 0; i < w.size(); ++i){
        wSum += w[i];
    }
}

void RungeKutta::update(MCS& mcs, float dt){
	if (mcs.getNumberOfConnections() > delta_p_offsets.size() ||
		mcs.getNumberOfParticles() > ka[0].size()){
		allocMemoryFor(mcs);
	}

	int p1, p2;
	//For each weight
	for (int k = 0; k < w.size(); ++k){

        //For each connection: calculate offset
        if (k>0){
            for (int i = 0; i < mcs.getNumberOfConnections(); ++i){
                p1 = mcs.connections.particle1[i];
                p2 = mcs.connections.particle2[i];
                delta_v_offsets[i] = (ka[k-1][p1] - ka[k-1][p2]) * dt / w[k];
                delta_p_offsets[i] = delta_v_offsets[i] * dt / w[k];
            }
        }

        //For each particle: ka and kv
        mcs.calcConnectionForcesOnParticles(delta_v_offsets, delta_p_offsets);
        for (int i = 0; i < mcs.getNumberOfParticles(); ++i){
            mcs.calcAccelerationOfParticle(i);
            ka[k][i] = mcs.particles.accelerations[i];
            kv[k][i] = mcs.particles.velocities[i] + ka[k][i] * dt;
        }
    }

    //Update positions and velocities
    glm::vec3 new_position, new_velocity;
    glm::vec3 delta_v, delta_p;
    for (int i = 0; i < mcs.getNumberOfParticles(); ++i){
        if (!mcs.lock_.particleLocked(i)){ // Detta är inte snyggt, numerical methods borde separeras mer från MCS. Gäller även kollisionen?

    		delta_v = glm::vec3(0,0,0);
    		delta_p = glm::vec3(0,0,0);
	        //Calc new position and velocity
        	for (int wi = 0; wi < w.size(); ++wi){
                delta_v += ka[wi][i] * w[wi];
                delta_p += kv[wi][i] * w[wi];
            }
            delta_v *= dt/wSum;
            delta_p *= dt/wSum;

            new_velocity = mcs.particles.velocities[i] + delta_v;
            new_position = mcs.particles.positions[i] + delta_p;

            //Check collisions with collision planes
            mcs.checkCollisions(new_position, new_velocity);
            
            //Set new position and velocity
            mcs.particles.velocities[i] = new_velocity;
            mcs.particles.positions[i] = new_position;
        }
    }
}

void RungeKutta::allocMemoryFor(const MCS& mcs){
	delta_v_offsets = std::vector<glm::vec3>(mcs.getNumberOfConnections(),glm::vec3(0,0,0));
    delta_p_offsets = std::vector<glm::vec3>(mcs.getNumberOfConnections(),glm::vec3(0,0,0));
    ka = std::vector<std::vector<glm::vec3> >(w.size());
    kv = std::vector<std::vector<glm::vec3> >(w.size());
    for (int i = 0; i < w.size(); ++i){
        ka[i] = std::vector<glm::vec3>(mcs.getNumberOfParticles(), glm::vec3(0,0,0));
        kv[i] = std::vector<glm::vec3>(mcs.getNumberOfParticles(), glm::vec3(0,0,0));
    }
}