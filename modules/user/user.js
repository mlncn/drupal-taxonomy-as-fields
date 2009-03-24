// $Id: user.js,v 1.11 2009/03/13 23:15:09 webchick Exp $
(function($) {

/**
 * Attach handlers to evaluate the strength of any password fields and to check
 * that its confirmation is correct.
 */
Drupal.behaviors.password = {
  attach: function(context, settings) {
    var translate = settings.password;
    $("input.password-field:not(.password-processed)", context).each(function() {
      var passwordInput = $(this).addClass('password-processed');
      var innerWrapper = $(this).parent();
      var outerWrapper = $(this).parent().parent();

      // Add the password strength layers.
      var passwordStrength = $("span.password-strength", innerWrapper);
      var passwordResult = $("span.password-result", passwordStrength);
      innerWrapper.addClass("password-parent");

      // Add the description box at the end.
      var passwordMeter = '<div id="password-strength"><div class="password-strength-title">' + translate.strengthTitle + '</div><div id="password-indicator"><div id="indicator"></div></div></div>';
      $("div.description", outerWrapper).prepend('<div class="password-suggestions"></div>');
      $(innerWrapper).append(passwordMeter);
      var passwordDescription = $("div.password-suggestions", outerWrapper).hide();

      // Add the password confirmation layer.
      $("input.password-confirm", outerWrapper).after('<div class="password-confirm">' + translate["confirmTitle"] + ' <span></span></div>').parent().addClass("confirm-parent");
      var confirmInput = $("input.password-confirm", outerWrapper);
      var confirmResult = $("div.password-confirm", outerWrapper);
      var confirmChild = $("span", confirmResult);

      // Check the password strength.
      var passwordCheck = function () {

        // Evaluate the password strength.
        var result = Drupal.evaluatePasswordStrength(passwordInput.val(), settings.password);

        // Update the suggestions for how to improve the password.
        if (passwordDescription.html() != result.message) {
          passwordDescription.html(result.message);
        }

        // Only show the description box if there is a weakness in the password.
        if (result.strength == 100) {
          passwordDescription.hide();
        }
        else {
          passwordDescription.show();
        }

        // Adjust the length of the strength indicator.
        $("#indicator").css('width', result.strength + '%');

        passwordCheckMatch();
      };

      // Check that password and confirmation inputs match.
      var passwordCheckMatch = function () {

        if (confirmInput.val()) {
          var success = passwordInput.val() === confirmInput.val();

          // Show the confirm result.
          confirmResult.css({ visibility: "visible" });

          // Remove the previous styling if any exists.
          if (this.confirmClass) {
            confirmChild.removeClass(this.confirmClass);
          }

          // Fill in the success message and set the class accordingly.
          var confirmClass = success ? "ok" : 'error';
          confirmChild.html(translate["confirm" + (success ? "Success" : "Failure")]).addClass(confirmClass);
          this.confirmClass = confirmClass;
        }
        else {
          confirmResult.css({ visibility: "hidden" });
        }
      };

      // Monitor keyup and blur events.
      // Blur must be used because a mouse paste does not trigger keyup.
      passwordInput.keyup(passwordCheck).focus(passwordCheck).blur(passwordCheck);
      confirmInput.keyup(passwordCheckMatch).blur(passwordCheckMatch);
    });
  }
};

/**
 * Evaluate the strength of a user's password.
 *
 * Returns the estimated strength and the relevant output message.
 */
Drupal.evaluatePasswordStrength = function (password, translate) {
  var weaknesses = 0, strength = 100, msg = [];

  var hasLowercase = password.match(/[a-z]+/);
  var hasUppercase = password.match(/[A-Z]+/);
  var hasNumbers = password.match(/[0-9]+/);
  var hasPunctuation = password.match(/[^a-zA-Z0-9]+/);

  // If there is a username edit box on the page, compare password to that, otherwise
  // use value from the database.
  var usernameBox = $("input.username");
  var username = (usernameBox.length > 0) ? usernameBox.val() : translate.username;

  // Lose 10 points for every character less than 6.
  if (password.length < 6) {
    msg.push(translate.tooShort);
    strength -= (6 - password.length) * 10;
  }

  // Count weaknesses.
  if (!hasLowercase) {
    msg.push(translate.addLowerCase);
    weaknesses++;
  }
  if (!hasUppercase) {
    msg.push(translate.addUpperCase);
    weaknesses++;
  }
  if (!hasNumbers) {
    msg.push(translate.addNumbers);
    weaknesses++;
  }
  if (!hasPunctuation) {
    msg.push(translate.addPunctuation);
    weaknesses++;
  }

  // Apply penalty for each weakness (balanced against length penalty).
  switch (weaknesses) {
    case 1:
      strength -= 12.5;
      break;

    case 2:
      strength -= 25;
      break;

    case 3:
      strength -= 40;
      break;

    case 4:
      strength -= 40;
      break;
  }

  // Check if password is the same as the username.
  if ((password !== '') && (password.toLowerCase() === username.toLowerCase())){
    msg.push(translate.sameAsUsername);
    // Passwords the same as username are always very weak.
    strength = 5;
  }

  // Assemble the final message.
  msg = translate.hasWeaknesses + "<ul><li>" + msg.join("</li><li>") + "</li></ul>";
  return { strength: strength, message: msg };
};

/**
 * On the admin/user/settings page, conditionally show all of the
 * picture-related form elements depending on the current value of the
 * "Picture support" radio buttons.
 */
Drupal.behaviors.userSettings = {
  attach: function(context, settings) {
    $('div.user-admin-picture-radios input[type=radio]:not(.userSettings-processed)', context).addClass('userSettings-processed').click(function () {
      $('div.user-admin-picture-settings', context)[['hide', 'show'][this.value]]();
    });
  }
};

})(jQuery);
