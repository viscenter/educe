function X_reg=greensite_l_curve(A,Y,trunc_deg)
% Function to calculate the Greensite inverse solution
%   A - forward matrix
%   Y - data
%   trunc_deg - truncation degree
%   X_reg - inverse solution


% set the default truncation degree to 10
if nargin < 3
    trunc_deg = 10;
end

% 'whiten' the data using SVD
% use reg_tools if available
if exist('csvd')
    [U,S,V]=csvd(Y'*Y);
    [Ua,Sa,Va]=csvd(A,0);        
    
% otherwise use built in SVD
else
    [U,S,V]=svd(Y'*Y);
    [Ua,Sa,Va]=svd(A,0);
end

trans_Y=Y*U;
sa=diag(Sa);
Ua_trans_Y=Ua'*trans_Y;

% calculate regularized solution for each frame
% until the truncation degree is reached
for i=1:trunc_deg
    % use l-curve if reg_tools is available
    if exist('l_curve')
        lambda(i)=l_curve(Ua,sa,trans_Y(:,i));
    % set default regularization parameter to .01
    else
        lambda(i) = .01;
    end
    
    X_reg(:,i)=Va*diag(sa./(sa.^2+lambda(i)^2))*Ua_trans_Y(:,i);
end

% recorrelate the solution
X_reg=X_reg*U(:,1:trunc_deg)';
    
