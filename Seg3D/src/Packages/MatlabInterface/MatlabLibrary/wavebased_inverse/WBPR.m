function [x_WBPR_forward,x_WBPR_backward] = WBPR(A,y,heart,first_act,last_act)
% Function to calculate the WBPR solutions
% Inputs:
%   A: forward matrix
%   y: torso data
%   heart: heart geometry
%   first_act: first activated node
%   last_act: last activated node
%
% Outputs:
%   x_WBPR_forward: inverse solution using forward WBPR
%   x_WBPR_backward: inverse solution using backward WBPR

% If reg_tools is installed
if exist('csvd')

    % get the neighbors matrix
    neighb_array = gen_neighb_array(heart.node',heart.face');
    QRS_length = size(y,2);

    % get the reference potential
    [epi_pot,ref]=remove_ref(y,1,QRS_length,first_act,last_act);
    
    % calculate the WBPR forward and backward solutions
    [x_WBPR_forward,x_WBPR_backward]=WBPR_main(A,y,heart.face',heart.node',neighb_array,1:QRS_length-1,first_act,last_act,QRS_length-1,ref);

else
    error('reg_tools required for the WBPR solution. See:\n[SCIRun]/src/Packages/MatlabInterface/MatlabLibrary/regtools/README.txt\nfor more information.','a');
end
