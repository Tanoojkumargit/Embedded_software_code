clc, clear, close all

% Initialize the baseline
b = 0.14; % meters

% Initialization Phase
iter = 0; T = 0;

% Initial robot states
xr = 0; yr = 0; thr = 0;

% Initial states vectors
Xr = xr; Yr = yr; THr = thr;

% Simulation Parameters
thetaplot = linspace(0, 2 * pi, 180);
rlength = 0.06;
rwidth = 0.14;
ts = 0.1;
tstop = 20;

% Set the control inputs (velocity profiles)
v1 = 0.8; w1 = 0.8; % Velocity profile i
v2 = 0.4; w2 = 0.4; % Velocity profile ii
v3 = 0.4; w3 = 0.8; % Velocity profile iii
v4 = 0.8; w4 = 0.4; % Velocity profile iv

while T(end) <= tstop
    figure(1), clf, hold on
    set(gcf, 'color', [1, 1, 1])
    set(gca, 'fontname', 'times', 'fontsize', 14, 'box', 'on')
    xlabel('x [m]', 'fontname', 'times', 'fontsize', 14)
    ylabel('y [m]', 'fontname', 'times', 'fontsize', 14)

    % Choose velocity profile based on iteration
    if mod(iter, 4) == 0
        v = v1; w = w1;
    elseif mod(iter, 4) == 1
        v = v2; w = w2;
    elseif mod(iter, 4) == 2
        v = v3; w = w3;
    else
        v = v4; w = w4;
    end

    % Update the robot's state using the predefined control inputs
    [tsolr, xsolr] = ode45(@DifferentialDriveKinematics, [iter * ts, (iter + 1) * ts], [xr, yr, thr]', [], [v, w]);
    xsolr = xsolr(end, :);
    tsolr = tsolr(end);
    xr = xsolr(1);
    yr = xsolr(2);
    thr = xsolr(3);

    Xr = [Xr; xr];
    Yr = [Yr; yr];
    THr = [THr; thr];
    T = [T; (iter + 1) * ts];

    plot(Xr, Yr, 'b-', 'linewidth', 2)
    fill(xr + cos(thr) * rlength * cos(thetaplot) - sin(thr) * rwidth * sin(thetaplot), ...
        yr + sin(thr) * rlength * cos(thetaplot) + cos(thr) * rwidth * sin(thetaplot), [0 0 0.8], 'EdgeColor', 'none', 'FaceAlpha', 0.8);

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * rwidth, xr + cos(thr) * (rlength / 3) - sin(thr) * rwidth], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * rwidth, yr + sin(thr) * (rlength / 3) + cos(thr) * rwidth], 'k', 'linewidth', 5);

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * (-rwidth), xr + cos(thr) * (rlength / 3) - sin(thr) * (-rwidth)], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * (-rwidth), yr + sin(thr) * (rlength / 3) + cos(thr) * (-rwidth)], 'k', 'linewidth', 5);

    title(sprintf('Robot Position -\nCurrent time [sec]: %.2f', T(end)), 'fontname', 'times', 'fontsize', 14)

    axis tight, axis equal
    pause(0.01);

    iter = iter + 1;
end
function dxdt = DifferentialDriveKinematics(t, x, v, w)
    % DifferentialDriveKinematics computes the derivative of the state vector for a differential drive robot.
    % t: time
    % x: state vector [x, y, theta]
    % v: linear velocity
    % w: angular velocity

    % Extract state variables
    xr = x(1);
    yr = x(2);
    thr = x(3);

    % Compute the derivative of the state vector
    dxdt = [
        v * cos(thr) - w * b * sin(thr);
        v * sin(thr) + w * b * cos(thr);
        w
    ];
end

