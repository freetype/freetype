/*! jqueryanimatesprite - v1.3.5 - 2014-10-17
* http://blaiprat.github.io/jquery.animateSprite/
* Copyright (c) 2014 blai Pratdesaba; Licensed MIT */
(function ($, window, undefined) {

    'use strict';
    var init = function (options) {

        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');

            // ASYNC
            // If we don't specify the columns, we
            // can discover using the background size
            var discoverColumns = function (cb) {
                var imageSrc = $this.css('background-image').replace(/url\((['"])?(.*?)\1\)/gi, '$2');
                var image = new Image();

                image.onload = function () {
                    var width = image.width,
                        height = image.height;
                    cb(width, height);
                };
                image.src = imageSrc;
            };

            if (!data) {
                $this.data('animateSprite', {
                    settings: $.extend({
                        width: $this.width(),
                        height: $this.height(),
                        totalFrames: false,
                        columns: false,
                        fps: 12,
                        complete: function () {},
                        loop: false,
                        autoplay: true
                    }, options),
                    currentFrame: 0,
                    controlAnimation: function () {

                        var checkLoop = function (currentFrame, finalFrame) {
                            currentFrame++;
                            if (currentFrame >= finalFrame) {
                                if (this.settings.loop === true) {
                                    currentFrame = 0;
                                    data.controlTimer();
                                } else {
                                    this.settings.complete();
                                }
                            } else {
                                data.controlTimer();
                            }
                            return currentFrame;
                        };

                        if (this.settings.animations === undefined) {
                            $this.animateSprite('frame', this.currentFrame);
                            this.currentFrame = checkLoop.call(this, this.currentFrame, this.settings.totalFrames);

                        } else {
                            if (this.currentAnimation === undefined) {
                                for (var k in this.settings.animations) {
                                    this.currentAnimation = this.settings.animations[k];
                                    break;
                                }
                            }
                            var newFrame  = this.currentAnimation[this.currentFrame];

                            $this.animateSprite('frame', newFrame);
                            this.currentFrame = checkLoop.call(this, this.currentFrame, this.currentAnimation.length);

                        }

                    },
                    controlTimer: function () {
                        // duration overrides fps
                        var speed = 1000 / data.settings.fps;

                        if (data.settings.duration !== undefined) {
                            speed = data.settings.duration / data.settings.totalFrames;
                        }

                        data.interval = setTimeout(function () {
                            data.controlAnimation();
                        }, speed);

                    }
                });


                data = $this.data('animateSprite');

                // Setting up columns and total frames
                if (!data.settings.columns) {
                    // this is an async function
                    discoverColumns(function (width, height) {
                        // getting amount of columns
                        data.settings.columns = Math.round(width / data.settings.width);
                        // if totalframes are not specified
                        if (!data.settings.totalFrames) {
                            // total frames is columns times rows
                            var rows = Math.round(height / data.settings.height);
                            data.settings.totalFrames = data.settings.columns * rows;
                        }
                        if (data.settings.autoplay) {
                            data.controlTimer();
                        }
                    });
                } else {

                    // if everything is already set up
                    // we start the interval
                    if (data.settings.autoplay) {
                        data.controlTimer();
                    }
                }


            }

        });

    };

    var frame = function (frameNumber) {
        // frame: number of the frame to be displayed
        return this.each(function () {
            if ($(this).data('animateSprite') !== undefined) {
                var $this = $(this),
                    data  = $this.data('animateSprite'),
                    row = Math.floor(frameNumber / data.settings.columns),
                    column = frameNumber % data.settings.columns;

                $this.css('background-position', (-data.settings.width * column) + 'px ' + (-data.settings.height * row) + 'px');
            }
        });
    };

    var stop = function () {
        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');
            clearTimeout(data.interval);
        });
    };

    var resume = function () {
        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');

            // always st'op animation to prevent overlapping intervals
            $this.animateSprite('stopAnimation');
            data.controlTimer();
        });
    };

    var restart = function () {
        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');

            $this.animateSprite('stopAnimation');

            data.currentFrame = 0;
            data.controlTimer();
        });
    };

    var play = function (animationName) {
        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');

            if (typeof animationName === 'string') {

                $this.animateSprite('stopAnimation');
                if (data.settings.animations[animationName] !== data.currentAnimation) {
                    data.currentFrame = 0;
                    data.currentAnimation = data.settings.animations[animationName];
                }
                data.controlTimer();
            } else {
                $this.animateSprite('stopAnimation');
                data.controlTimer();
            }

        });
    };

    var fps = function (val) {
        return this.each(function () {
            var $this = $(this),
                data  = $this.data('animateSprite');
            // data.fps
            data.settings.fps = val;
        });
    };

    var methods = {
        init: init,
        frame: frame,
        stop: stop,
        resume: resume,
        restart: restart,
        play: play,
        stopAnimation: stop,
        resumeAnimation: resume,
        restartAnimation: restart,
        fps: fps
    };

    $.fn.animateSprite = function (method) {

        if (methods[method]) {
            return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
        } else if (typeof method === 'object' || ! method) {
            return methods.init.apply(this, arguments);
        } else {
            $.error('Method ' +  method + ' does not exist on jQuery.animateSprite');
        }

    };

})(jQuery, window);
