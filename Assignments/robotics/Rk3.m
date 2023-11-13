clc, clear, close all
% Clear the command window, workspace, and close all open figures.

% Define trajectory parameters
a = 1;
k = 3;
c = 0.1;
% Set parameters for the desired trajectory.

% Initialization Phase
iter = 0;
T = 0;
% Initialize iteration counter and time variable.

% Initial robot states
xr = 0;
yr = 0;
thr = 0; % Initial heading angle
% Initialize the robot's position and orientation.

Xr = xr; % For trajectory tracking
Yr = yr; % For trajectory tracking
THr = thr; % For trajectory tracking
% Create vectors to store the robot's trajectory.

% Simulation Parameters
thetaplot = linspace(0, 2 * pi, 180);
rlength = 0.06;
rwidth = 0.14;
ts = 0.05;
tstop = 30; % Simulation time
% Define simulation parameters, such as angles, robot dimensions, time step, and simulation stop time.

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

    % Calculate the desired trajectory based on the given equations
    t = iter * ts;
    x_desired = a * cos(k * c * t) * cos(c * t);
    y_desired = a * cos(k * c * t) * sin(c * t);
    % Compute the desired x and y coordinates for the robot's trajectory.

    % Calculate the control inputs (inverse kinematics)
    v = sqrt(x_desired^2 + y_desired^2) / ts; % Linear velocity
    w = (x_desired * cos(thr) + y_desired * sin(thr)) / (a * cos(k * c * t)) * ts; % Angular velocity
    % Compute control inputs (linear and angular velocities) for the robot to follow the desired trajectory.

    % Update the robot's state using the calculated control inputs
    [tsolr, xsolr] = ode45(@DifferentialDriveKinematics, [t, t + ts], [xr, yr, thr]', [], [v, w]);
    xsolr = xsolr(end, :);
    tsolr = tsolr(end);
    xr = xsolr(1);
    yr = xsolr(2);
    thr = xsolr(3);
    % Update the robot's position and orientation based on the differential equation.

    Xr = [Xr; xr]; % Store the trajectory
    Yr = [Yr; yr]; % Store the trajectory
    THr = [THr; thr]; % Store the trajectory
    T = [T; t + ts];
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
