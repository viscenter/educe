function nij_act_times = interp_nij_act_times(nij_proj_pts,pts,nearest_pts,activation_times);

% nij_proj_pts is projected geometry
% pts is original geometry
% nearest_pts are the triangle projected onto
% activation_times are the act times for the original data
% nij_act_times are the output activation times (linear interpolation)

nij_act_times = [];

for i = 1:size(nij_proj_pts,1)
    
    pi = nij_proj_pts(i,:);
    p1 = pts(nearest_pts(i,1),:);
    p2 = pts(nearest_pts(i,2),:);
    p3 = pts(nearest_pts(i,3),:);
    
    d1 = sqrt(pi*p1');
    d2 = sqrt(pi*p2');
    d3 = sqrt(pi*p3');
    dt = d1+d2+d3;
    
    a1 = activation_times(nearest_pts(i,1));
    a2 = activation_times(nearest_pts(i,2));
    a3 = activation_times(nearest_pts(i,3));
   
    %v = [a1;a2;a3];
    
    %vi = interp3(x,y,z,v,xi,yi,zi);
    
    vi = a1*(dt-2*d1)/dt + a2*(dt-2*d2)/dt + a3*(dt-2*d3)/dt;
    
    nij_act_times = [nij_act_times;vi];
end

    
    