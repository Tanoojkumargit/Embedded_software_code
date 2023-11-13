clc, clear, close all
% Clear the command window, workspace, and close all open figures.

% Initialize the baseline
b = 0.14; % meters
% Set the baseline parameter to 0.14 meters.

% Initialization Phase
iter = 0; T = 0;
% Initialize iteration counter and time variable.

% Initial robot states
xr = 0; yr = 0; thr = 0;
% Initialize the robot's position and heading angle.

% Initial states vectors
Xr = xr; Yr = yr; THr = thr;
% Create vectors to store the robot's trajectory.

% Simulation Parameters
thetaplot = linspace(0, 2 * pi, 180);
rlength = 0.06;
rwidth = 0.14;
ts = 0.1;
tstop = 20;
% Define simulation parameters, such as 4, robot dimensions, time step, and simulation stop time.

% Set the control inputs (velocity profiles)
v = 0.8; % m/s
w = 0.4; % rad/s
% Set constant linear and angular velocity profiles for the robot.

while T(end) <= tstop
    % Start a loop that runs until the simulation time reaches the specified stop time.
    
    figure(1), clf, hold on
    % Create a figure with index 1, clear the figure, and start plotting.
    set(gcf, 'color', [1, 1, 1])
    set(gca, 'fontname', 'times', 'fontsize', 14, 'box', 'on')
    % Set figure and axis properties, such as background color, font, and axis appearance.
    
    xlabel('x [m]', 'fontname', 'times', 'fontsize', 14)
    ylabel('y [m]', 'fontname', 'times', 'fontsize', 14)
    % Label the x and y axes.

    % Update the robot's state using the predefined control inputs
    [tsolr, xsolr] = ode45(@DifferentialDriveKinematics, [iter * ts, (iter + 1) * ts], [xr, yr, thr]', [], [v, w]);
    xsolr = xsolr(end, :);
    tsolr = tsolr(end);
    xr = xsolr(1);
    yr = xsolr(2);
    thr = xsolr(3);
    % Update the robot's position and orientation based on the differential equation.

    Xr = [Xr; xr]; % Store the trajectory
    Yr = [Yr; yr]; % Store the trajectory
    THr = [THr; thr]; % Store the trajectory
    T = [T; (iter + 1) * ts];
    % Store the robot's position, orientation, and time in vectors.

    % Plot the robot's path
    plot(Xr, Yr, 'b-', 'linewidth', 2)
    fill(xr + cos(thr) * rlength * cos(thetaplot) - sin(thr) * rwidth * sin(thetaplot), ...
        yr + sin(thr) * rlength * cos(thetaplot) + cos(thr) * rwidth * sin(thetaplot), [0 0 0.8], 'EdgeColor', 'none', 'FaceAlpha', 0.8);
    % Plot the robot's trajectory and shape.

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * rwidth, xr + cos(thr) * (rlength / 3) - sin(thr) * rwidth], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * rwidth, yr + sin(thr) * (rlength / 3) + cos(thr) * rwidth], 'k', 'linewidth', 5);
    % Draw the robot's wheels.

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * (-rwidth), xr + cos(thr) * (rlength / 3) - sin(thr) * (-rwidth)], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * (-rwidth), yr + sin(thr) * (rlength / 3) + cos(thr) * (-rwidth)], 'k', 'linewidth', 5);
    % Draw the axes of the robot's wheels.

    title(sprintf('Robot Position -\nCurrent time [sec]: %.2f', T(end)), 'fontname', 'times', 'fontsize', 14)
    % Set the title of the plot to show the current time.

    axis tight, axis equal
    % Set the axis limits to fit the plotted data and maintain an equal aspect ratio.

    pause(0.01);
    % Pause briefly to control the animation speed.

    iter = iter + 1;
    % Increment the iteration counter.
end
