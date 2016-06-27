var express = require('express');
var router = express.Router();


/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('more', { title: 'DTCCL' });
  console.log("about call");
});



module.exports = router;
