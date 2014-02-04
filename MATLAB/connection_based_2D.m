% Allokering av minne
clear all;
nRows = 4;
nCols = 4;

g = 9.82;

number_of_masses = nRows*nCols;%  -                |                 / and \
number_of_connections = (nRows-1)*nCols + nRows*(nCols-1) + 2*(nRows-1)*(nCols-1);

%--Masses list--
masses = ones(number_of_masses,1);
positions = zeros([number_of_masses, 2, 2]);
velocities = zeros([number_of_masses, 2, 2]);
forces = zeros([number_of_masses, 2]);

for j=1:number_of_masses % For each mass
    positions(j,:,1) = [floor((j-1)/nCols), rem(j-1,nCols)];
end

%Calculate connection2massIndices
connected_masses = zeros([number_of_connections, 2]);
for i=1:number_of_connections
    [mass_index1, mass_index2] = connection2massIndices(i,nRows,nCols);
    connected_masses(i,1) = mass_index1;
    connected_masses(i,2) = mass_index2;
end

%--Connections list--
spring_constants = 2000 * ones(number_of_connections, 1);
damper_constants = 5 * ones(number_of_connections, 1);
spring_length = ones(number_of_connections, 1);

spring_length(nRows*(nCols-1)+1:nRows*(nCols-1) + (nRows-1)*(nCols-1)) = sqrt(2);
spring_length(nRows*(nCols-1) + (nRows-1)*(nCols-1) + (nRows-1)*nCols + 1:number_of_connections) = sqrt(2);


%% Startvärden. Simulering
close all;

read_buffer_index = 1;
write_buffer_index = 2;

velocities(:,:,:) = 0;
for j=1:number_of_masses % For each mass
    positions(j,:,read_buffer_index) = [15 + floor((j-1)/nCols), rem(j-1,nCols)];
end

%Add rotation to the body
%velocities(1,:,read_buffer_index) = [5, -5];
%velocities(number_of_masses,:,read_buffer_index) = [-5, 5];

n_frames = 300;
T = 0.01;

figure;
%pause(2);


for i=1:n_frames %Loop through frames
	tic;
    hold on;
    
    for connection_index=1:number_of_connections %Loop through connections
        
        % Spring and damper properties
        k = spring_constants(connection_index);
        b = damper_constants(connection_index);
        l = spring_length(connection_index);
        
        % position
        mass_index1 = connected_masses(connection_index,1);
        mass_index2 = connected_masses(connection_index,2);
        p1 = positions(mass_index1,:,read_buffer_index);
        p2 = positions(mass_index2,:,read_buffer_index);
        delta_p = p1 - p2;
        
        %Normalize delta_p
        norm_delta_p = norm(delta_p);
        if(norm_delta_p == 0)
            delta_p_hat = [0,0];
        else
            delta_p_hat = delta_p/norm_delta_p;
        end
        
        % velocities
        v1 = velocities(mass_index1,:,read_buffer_index);
        v2 = velocities(mass_index2,:,read_buffer_index);
        delta_v = v1 - v2;
        
        %Call numerical method
        %Euler: F = F + (-k*(norm(deltaP) - l) - b*dot(deltaV, delta_P_hat))*delta_P_hat;
        
        %force_from_connection = [0,0]; % = numerical_method(k,l,b,delta_p,delta_v,T)
        force_from_connection = (-k*(norm(delta_p) - l) - b*dot(delta_v, delta_p_hat))*delta_p_hat;
        forces(mass_index1,:) = forces(mass_index1,:) + force_from_connection;
        forces(mass_index2,:) = forces(mass_index2,:) - force_from_connection;

    end
    
    %Calculacte acceleration, velocity and position
    for mass_index=1:number_of_masses
        
        %   connection forces                       gravity
        a = forces(mass_index,:)/masses(mass_index) + g*[-1,0];
        v = velocities(mass_index, :,read_buffer_index) + T*a;
        p = positions(mass_index, :,read_buffer_index) + T*v;
        
        
        %Check collision with y=0
        if p(1) < 0
            p(1) = 0;
            v(1) = -v(1);
        end
        
        %Store information in backbuffer
        velocities(mass_index, :,write_buffer_index) = v;
        positions(mass_index, :,write_buffer_index) = p;
        
        %reset force
        forces(mass_index,:) = [0,0];
    end
        
    plot(positions(:,2,write_buffer_index), ...
            positions(:,1,write_buffer_index),'*');
    %axis manual;
    axis equal;
    axis([-2 5 0 20]);
    computation_time = toc;
    pause(max(T-computation_time,0.001));
    
    %Swap buffer
    read_buffer_index = rem(read_buffer_index,2)+1;
    write_buffer_index = rem(write_buffer_index,2)+1;
    clf('reset');
    %toc;
end
