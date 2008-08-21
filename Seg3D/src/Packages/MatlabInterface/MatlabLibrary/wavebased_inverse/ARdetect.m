function x = ARdetect(sig,win,deg,pol,ndrange)
 
   if nargin == 4,
        ndrange = 0;
   end
 
   sigdrange = max(sig)-min(sig);
   
   if (sigdrange <= 1.75*ndrange),
        x = length(sig);
        return;
   end
   
   if mod(win,2) == 0, win = win + 1; end
   if length(sig) < win, x=1; return; end
 
    % Detection of the minimum derivative
    % Use a window of 5 frames and fit a 2nd order polynomial
    
    cen = ceil(win/2);
    X = zeros(win,(deg+1));
    L = [-(cen-1):(cen-1)]'; for p=1:(deg+1), X(:,p) = L.^((deg+1)-p); end
    E = inv(X'*X)*X';

    sig = [sig sig(end)*ones(1,cen-1)];
    
    a = filter(E(deg,[win:-1:1]),1,sig);
    dy = a(cen:end);

    if pol == 1,
        [mv,mi] = min(dy(cen:end-cen));
    else
        [mv,mi] = max(dy(cen:end-cen));
    end
    mi = mi(1)+(cen-1);
    
    % preset values for peak detector
    
    win2 = 5;
    deg2 = 2;
    
    cen2 = ceil(win2/2);
    L2 = [-(cen2-1):(cen2-1)]';
    for p=1:(deg2+1), X2(:,p) = L2.^((deg2+1)-p); end
    c = inv(X2'*X2)*X2'*(dy(L2+mi)');
    
    if abs(c(1)) < 100*eps, dx = 0; else dx = -c(2)/(2*c(1)); end
   
    dx = median([-0.5 dx 0.5]);
    
    x = mi+dx-1;
    
    return