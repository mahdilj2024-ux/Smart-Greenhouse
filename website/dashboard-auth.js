/* ============================================================
   Dashboard auth helper: logout button behavior.
   The redirect-if-not-logged-in check runs earlier, inline in
   <head>, so protected content never flashes on screen.
   ============================================================ */
document.addEventListener('DOMContentLoaded', function () {
    var logoutBtn = document.getElementById('logoutBtn');
    if (!logoutBtn) return;

    logoutBtn.addEventListener('click', function () {
        sessionStorage.removeItem('gh-auth');
        sessionStorage.removeItem('gh-user');
        window.location.href = 'index.html';
    });

    var userLabel = document.getElementById('currentUser');
    if (userLabel) {
        userLabel.textContent = sessionStorage.getItem('gh-user') || '';
    }
});
