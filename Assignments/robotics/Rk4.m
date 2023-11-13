clc, clear, close all
% Clear the command window, workspace, and close all open figures.

% Initialize the baseline
b = 0.14; % meters
% Set the baseline parameter to 0.14 meters.

% Initialization Phase
iter = 0;
T = 0;
% Initialize iteration counter and time variable.

% Initial robot states
xr = 0; yr = 0; thr = 0;
% Initialize the robot's position and heading angle.

% Initialize dead reckoning pose estimate
x_dr = xr;
y_dr = yr;
th_dr = thr;
% Initialize the dead reckoning estimated position and heading.

% Initial states vectors
Xr = xr; Yr = yr; THr = thr;
% Create vectors to store the robot's trajectory.

% Initialize error variables
error_x = 0;
error_y = 0;
error_th = 0;
% Initialize variables to track the error between estimated and actual position.

% Simulation Parameters
thetaplot = linspace(0, 2 * pi, 180);
rlength = 0.06;
rwidth = 0.14;
ts = 0.05;
tstop = 20;
% Define simulation parameters, such as angles, robot dimensions, time step, and simulation stop time.

% Set the control inputs (velocity profiles)
v = 0.8; % m/s
w = 0.8; % rad/s
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

    % Dead reckoning update
    x_dr = x_dr + v * cos(th_dr) * ts;
    y_dr = y_dr + v * sin(th_dr) * ts;
    th_dr = th_dr + w * ts;
    % Update the dead reckoning estimated position and heading based on control inputs.

    [tsolr, xsolr] = ode45(@DifferentialDriveKinematics, [iter * ts, (iter + 1) * ts], [xr, yr, thr]', [], [v, w]);
    xsolr = xsolr(end, :);
    tsolr = tsolr(end);
    xr = xsolr(1);
    yr = xsolr(2);
    thr = xsolr(3);
    % Update the robot's position and orientation based on the differential equation.

    % Update states vectors
    Xr = [Xr; xr];
    Yr = [Yr; yr];
    THr = [THr; thr];
    T = [T; (iter + 1) * ts];
    % Store the robot's position, orientation, and time in vectors.

    % Calculate and update error
    error_x = xr - x_dr;
    error_y = yr - y_dr;
    error_th = thr - th_dr;
    % Calculate the error between the actual and estimated positions.

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

    % Plot the dead reckoning estimate (optional, for comparison)
    plot(x_dr, y_dr, 'r--', 'linewidth', 1.5);
    % Plot the dead reckoning estimated path.

    title(sprintf('Robot Position -\nCurrent time [sec]: %.2f', T(end)), 'fontname', 'times', 'fontsize', 14)
    % Set the title of the plot to show the current time.

    axis tight, axis equal
    % Set the axis limits to fit the plotted data and maintain an equal aspect ratio.

    % Print error values
    fprintf('Error_x: %.4f, Error_y: %.4f, Error_th: %.4f\n', error_x, error_y, error_th);
    % Print the error values between actual and estimated positions.

    pause(0.01);
    % Pause briefly to control the animation speed.

    iter = iter + 1;
    % Increment the iteration counter.
end
