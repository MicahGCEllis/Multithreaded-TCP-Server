// app.js
document.addEventListener('DOMContentLoaded', () => {
    const concurrencyStatus = document.getElementById('concurrency-status');
    const firewallBtn = document.getElementById('firewall-btn');
    const firewallStatus = document.getElementById('firewall-status');

    // Concurrency Testing
    const testConcurrency = async () => {
        concurrencyStatus.innerText = 'Running 20 requests...';
        const requests = [];
        
        for (let i = 0; i < 20; i++) {
            requests.push(
                fetch('/dummy-file-test-' + i + '.txt')
                    .then(res => res.status)
                    .catch(() => 'Failed')
            );
        }

        try {
            const results = await Promise.all(requests);
            const successCount = results.filter(status => status !== 'Failed').length;
            concurrencyStatus.innerText = `${successCount}/20 completed`;
        } catch (err) {
            concurrencyStatus.innerText = 'Test Error';
            concurrencyStatus.style.color = '#d32f2f';
        }
    };

    // Trigger concurrency test on load
    testConcurrency();

    // Firewall Testing
    firewallBtn.addEventListener('click', async () => {
        firewallStatus.innerText = 'Transmitting malicious payload...';
        firewallStatus.style.color = '#39ff14';

        try {
            const response = await fetch('/%2e%2e%2fWindows/System32');
            
            if (response.status === 403) {
                firewallStatus.innerText = '[SUCCESS] 403 Forbidden: Firewall blocked traversal.';
                firewallStatus.style.color = '#39ff14';
            } else {
                firewallStatus.innerText = `[WARNING] Server returned status: ${response.status}`;
                firewallStatus.style.color = '#d32f2f';
            }
        } catch (err) {
            firewallStatus.innerText = `[ERROR] Connection dropped or failed: ${err.message}`;
            firewallStatus.style.color = '#d32f2f';
        }
    });
});