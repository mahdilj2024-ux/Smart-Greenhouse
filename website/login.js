/* ============================================================
   Login handling
   NOTE: This is a client-side demo gate only (sessionStorage flag).
   For real protection, verify the credentials on a server/API
   instead of hardcoding them in the browser.
   ============================================================ */
document.addEventListener('DOMContentLoaded', function () {
    var form = document.getElementById('loginForm');
    var errorBox = document.getElementById('loginError');
    var card = document.getElementById('loginCard');
    var usernameInput = document.getElementById('username');
    var passwordInput = document.getElementById('password');

    var VALID_USERNAME = 'Mahdi';
    var VALID_PASSWORD = '1234';

    // If already logged in this session, skip straight to the dashboard.
    if (sessionStorage.getItem('gh-auth') === '1') {
        window.location.replace('dashboard.html');
        return;
    }

    form.addEventListener('submit', function (e) {
        e.preventDefault();

        var username = usernameInput.value.trim();
        var password = passwordInput.value;

        if (username === VALID_USERNAME && password === VALID_PASSWORD) {
            sessionStorage.setItem('gh-auth', '1');
            sessionStorage.setItem('gh-user', username);
            window.location.href = 'dashboard.html';
        } else {
            errorBox.textContent = 'Incorrect username or password. Please try again.';
            errorBox.classList.add('show');

            card.classList.remove('shake');
            void card.offsetWidth; /* restart animation */
            card.classList.add('shake');

            passwordInput.value = '';
            passwordInput.focus();
        }
    });

    var footerYear = document.getElementById('footerYear');
    if (footerYear) footerYear.textContent = new Date().getFullYear();
});
