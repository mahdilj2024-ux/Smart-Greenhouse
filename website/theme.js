/* ============================================================
   Shared theme toggle logic (Dark / Light mode)
   Used on both the login page and the dashboard.
   ============================================================ */
(function () {
    function applyIcon(theme) {
        var icon = document.getElementById('themeIcon');
        if (icon) icon.textContent = theme === 'light' ? '☀️' : '🌙';
    }

    var current = localStorage.getItem('gh-theme') || 'dark';
    applyIcon(current);

    var btn = document.getElementById('themeToggle');
    if (btn) {
        btn.addEventListener('click', function () {
            var next = document.documentElement.getAttribute('data-theme') === 'light' ? 'dark' : 'light';
            document.documentElement.setAttribute('data-theme', next);
            localStorage.setItem('gh-theme', next);
            applyIcon(next);
        });
    }
})();
