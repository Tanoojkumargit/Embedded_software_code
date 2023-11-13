clc, clear, close all
% Clear the command window, workspace, and close all open figures.

% Initialization
iter = 0;
T = 0;
% Initialize iteration counter and time variable.

% Initial robot states
xr = 0;
yr = 0;
thr = 0;
% Initialize the robot's position (x, y) and heading angle (theta).

Xr = xr;
Yr = yr;
THr = thr;
% Create vectors to store the robot's trajectory.

% Simulation parameters
thetaplot = linspace(0, 2 * pi, 180);
rlength = 0.1; % Update the robot's length to 0.1 meters.
rwidth = 0.14;
ts = 0.025; % Reduce the sampling time for better resolution.
tstop = 25; % Update the stop time to 25 seconds.

% Simulation Loop
while T(end) <= tstop
    % Start a loop that runs until the simulation time reaches the specified stop time.
    
    figure(1), clf, hold on
    % Create a figure with index 1, clear the figure, and start plotting.
    set(gcf, 'color', [1, 1, 1]);
    set(gca, 'fontname', 'times', 'fontsize', 14, 'box', 'on');
    xlabel('x [m]', 'fontname', 'times', 'fontsize', 14);
    ylabel('y [m]', 'fontname', 'times', 'fontsize', 14);
    % Set figure and axis properties, including colors and fonts.

    % Set the desired velocity profiles
    v1 = 0.2; % Constant linear velocity of 0.2 m/s.
    v2 = 0.05*cos(0.5*tstop); % Angular velocity as a function of time.
    % Update the linear and angular velocity profiles.

    % Real Robot
    [tsolr, xsolr] = ode45(@DifferentialDriveKinematics, [iter * ts, (iter + 1) * ts], [xr, yr, thr]', [], [v1, v2]);
    % Simulate the real robot's motion using a differential drive model.

    xsolr = xsolr(end, :);
    tsolr = tsolr(end);
    xr = xsolr(1);
    yr = xsolr(2);
    thr = xsolr(3);
    % Update the robot's position and orientation based on the simulation.

    Xr = [Xr; xr];
    Yr = [Yr; yr];
    THr = [THr; thr];
    T = [T; (iter + 1) * ts];
    % Store the robot's position, orientation, and time in vectors.

    % Plot Frame
    plot(Xr, Yr, 'b-', 'linewidth', 2);
    % Plot the robot's trajectory.

    fill(xr + cos(thr) * rlength * cos(thetaplot) - sin(thr) * rwidth * sin(thetaplot), ...
        yr + sin(thr) * rlength * cos(thetaplot) + cos(thr) * rwidth * sin(thetaplot), [0 0 0.8], 'EdgeColor', 'none', 'FaceAlpha', 0.8);
    % Draw the robot's shape.

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * rwidth, xr + cos(thr) * (rlength / 3) - sin(thr) * rwidth], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * rwidth, yr + sin(thr) * (rlength / 3) + cos(thr) * rwidth], 'k', 'linewidth', 5);
    % Draw the robot's wheels.

    plot([xr + cos(thr) * (-rlength / 3) - sin(thr) * (-rwidth), xr + cos(thr) * (rlength / 3) - sin(thr) * (-rwidth)], ...
        [yr + sin(thr) * (-rlength / 3) + cos(thr) * (-rwidth), yr + sin(thr) * (rlength / 3) + cos(thr) * (-rwidth)], 'k', 'linewidth', 5);
    % Draw the axes of the robot's wheels.

    title(sprintf('Robot Position - \n Current time [sec]: %.2f', T(end)), 'fontname', 'times', 'fontsize', 14);
    % Set the title of the plot to show the current time.

    axis tight;
    axis equal;
    % Set the axis limits to fit the plotted data and maintain an equal aspect ratio.

    pause(0.01);
    % Pause briefly to control the animation speed.

    % Next iteration
    iter = iter + 1;
    % Increment the iteration counter.
end
