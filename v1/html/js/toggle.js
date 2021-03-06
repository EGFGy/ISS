	var layout   = document.getElementById('layout'),
	menu     = document.getElementById('menu'),
	menuLink = document.getElementById('menuLink');

    function toggleClass(element, className) {
        var classes = element.className.split(/\s+/),
            length = classes.length,
            i = 0;

        for(; i < length; i++) {
          if (classes[i] === className) {
            classes.splice(i, 1);
            break;
          }
        }
        // The className is not found
        if (length === classes.length) {
            classes.push(className);
        }

        element.className = classes.join(' ');
    }

    function menuShowHide(e) {
        var active = 'active';

        //e.preventDefault();
        toggleClass(document.getElementById('layout'), active);
        toggleClass(document.getElementById('menu'), active);
        toggleClass(document.getElementById('menuLink'), active);
    };
