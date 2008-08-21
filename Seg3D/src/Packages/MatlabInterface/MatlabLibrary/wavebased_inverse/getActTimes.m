function act = getActTimes(x,pol)

    if nargin == 1
        pol = -1;
    end
    
    if pol > 0
        pol = 1;
    else
        pol = -1;
    end
    
    nleads = size(x,1);
    act = zeros(nleads,1);
    
    for i = 1:nleads
        act(i) = ARdetect(x(i,:),21,2,pol);
    end
    
    return