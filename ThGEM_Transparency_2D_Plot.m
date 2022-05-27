%Code to asses ThGEM transparency near the holes. 
%First we define some variables. All units of length are in micrometers
%(\mu m)
%positive y is towards PEN. 
clear();
clf;
invtwopi=0.159154943091895335768883763372514362034459645740456448747667344;

thickness = 1000;
radius = 200;
distance = 400;

%resolution of the grid
resolutionx = 10000; %must be a multiple of two
resolutiony = 10000;
transparency = zeros(resolutionx,resolutiony);
totalxdist = radius+(0.5*distance);
xincr = totalxdist/resolutionx;

%We compute the regions in which we have our different ways of evaluating
%the transparency based on geometrical considerations. 
yxslope = thickness/(2*radius);
xyslope = 1/yxslope;

miny=(distance+radius)*yxslope;
yincr = miny/resolutiony;
y1 = thickness+(yxslope*0.5*distance);

%iterate over the grid
for yidx=1:resolutiony
    for xidx=1:resolutionx
y=yidx*yincr;
x=xidx*xincr;

if (x<radius)
    theta1=atan((radius-x)/y); 
    theta2=atan((radius+x)/y);
    transparency(xidx,yidx)=(theta1+theta2)*invtwopi;
elseif(y<y1)
    if (x<(radius+(xyslope*(y-thickness))))
        theta1=atan((radius+x)/y);
        theta2=atan((x-radius)/(y-thickness));
        transparency(xidx,yidx)=(theta1-theta2)*invtwopi;
    end
elseif (x<(radius+distance-(xyslope*(y-thickness))))
    theta1=atan((radius+x)/y);
        theta2=atan((x-radius)/(y-thickness));
        transparency(xidx,yidx)=(theta1-theta2)*invtwopi;
end
    end
end
colormap('gray');
imagesc(transparency');
xlabel('Distance From Center of Hole (\mu m)');
ylabel('Distance From ThGEM Up (\mu m)');
xtickincr = resolutionx*0.1;
%xticks(0:xtickincr:resolutionx);
ytickincr = resolutiony*0.1;
%yticks(0:ytickincr:resolutiony);

xlblincr = totalxdist*0.1;
ylblincr=miny*0.1;



set(gca, 'XTick', [0:xtickincr:resolutionx], 'XTickLabel', [0:xlblincr:totalxdist]); 
set(gca, 'YTick', [0:xtickincr:resolutionx], 'YTickLabel', [0:ylblincr:miny]); 
title('ThGEM Transparency');
colorbar();

print(gcf(),'ThGEM transparency', '-dpng');